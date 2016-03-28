	gfx::Texture2DArray* tex_radiance = new gfx::Texture2DArray(r_device, WIDTH, HEIGHT, 6, 1, Format(Fmt_R32G32B32A32_Float, FmtView_Float), rend::TexUse_RenderTarget);
	gfx::Texture2D* tex_irradiance_cpu = new gfx::Texture2D(r_device, WIDTH, HEIGHT, 1, Format(Fmt_R8G8B8A8_UInt, FmtView_UNorm), rend::TexUse_CPUReadWrite);

	//   2
	// 1 4 0 5
	//   3
	math::vec3f face_directions[6] =
	{
		{  1,  0,  0 },
		{ -1,  0,  0 },
		{  0,  1,  0 },
		{  0, -1,  0 },
		{  0,  0,  1 },
		{  0,  0, -1 },
	};

	// Generate rotation quaternions for each face
	math::vec3f y = { 0, 1, 0 };
	math::vec3f z = { 0, 0, 1 };
	math::vec3f face_up[6] = { y, y, -z, z, y, y };
	math::quat face_rotations_q[6];
	for (u32 i = 0; i < 6; i++)
		face_rotations_q[i] = qFromMat3(m3Direction(face_directions[i], face_up[i]));

	// Render radiance textures for each face
	Viewport viewport(WIDTH, HEIGHT, 1, 100 * 1000, math::fPI() / 2.0f);
	for (u32 i = 0; i < 6; i++)
	{
		math::framed face_frame;
		face_frame.position = camera_frame.position;
		face_frame.rotation = face_rotations_q[i];
		RenderContext ctx(m_Profiler, m_Device, m_Resources, viewport, face_frame, nullptr);

		// TODO: Exempt sun
		m_SkyPass->Render(ctx, tex_radiance->RenderTarget(i), nullptr);
	}

	// Make a CPU copy of the incoming radiance
	gfx::Texture2DArray* tex_radiance_cpu = new gfx::Texture2DArray(r_device, WIDTH, HEIGHT, 6, 1, Format(Fmt_R32G32B32A32_Float, FmtView_Float), rend::TexUse_CPURead);
	tex_radiance_cpu->CopyFrom(tex_radiance);

	// Map all incoming radiance textures for read
	gfx::MappedTexture rad_maps[6];
	for (u32 i = 0; i < 6; i++)
		rad_maps[i] = tex_radiance_cpu->Map(rapi::MAP_Read, i, 0);

	// Save radiance textures to disk
	for (u32 i = 0; i < 6; i++)
	{
		img::Image image(WIDTH, HEIGHT, img::FORMAT_RGBA32, (u8*)rad_maps[i].data, img::DATAOWNER_External);
		core::String256 filename;
		filename.setv("c:/cubemap%d.tga", i);
		m_Resources->images->Save(filename.c_str(), image);
	}

	float accum_wt = 0;
	float sh_radiance_r[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	float sh_radiance_g[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	float sh_radiance_b[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	// Project cubemap to SH
	for (u32 i = 0; i < 6; i++)
	{
		gfx::MappedTexture map = rad_maps[i];
		if (map.data == nullptr)
			continue;
		float4* data = (float4*)map.data;

		// Iterate over texels of this face
		for (u32 y = 0; y < HEIGHT; y++)
		{
			for (u32 x = 0; x < WIDTH; x++)
			{
				// Generate a world-space ray for this texel
				float2 uv = TexelIndexToUV(x, y, WIDTH, HEIGHT);
				float3 ray = UVToRay(uv);
				math::vec3f vec_ray = { ray.x, ray.y, ray.z };
				vec_ray = math::qTransformPos(face_rotations_q[i], vec_ray);
				ray.x = vec_ray.x;
				ray.y = vec_ray.y;
				ray.z = vec_ray.z;

				// Evaluate SH along that ray
				float sh_dir[9];
				math::SH_Y2(ray, sh_dir);

				// Calcuate weighting from differential solid angle
				float u = uv.x * 2 - 1;
				float v = uv.y * 2 - 1;
				float tmp = 1 + u * u + v * v;
				float diff_solid_wt = 4 / (sqrtf(tmp) * tmp);
				accum_wt += diff_solid_wt;

				// Sum SH for each RGB channel
				float4 texel = data[y * WIDTH + x];
				math::SH_AddScaled2(sh_radiance_r, sh_dir, texel.x * diff_solid_wt);
				math::SH_AddScaled2(sh_radiance_g, sh_dir, texel.y * diff_solid_wt);
				math::SH_AddScaled2(sh_radiance_b, sh_dir, texel.z * diff_solid_wt);
			}
		}
	}

	// Normalise the SH for each RGB channel
	float normalise_sh = (4 * math::fPI()) / accum_wt;
	math::SH_Scale2(sh_radiance_r, normalise_sh);
	math::SH_Scale2(sh_radiance_g, normalise_sh);
	math::SH_Scale2(sh_radiance_b, normalise_sh);

	// Reconstruct cubemap from SH
	float4* reconstructed_radiance = new float4[WIDTH * HEIGHT];
	for (u32 i = 0; i < 6; i++)
	{
		for (u32 y = 0; y < HEIGHT; y++)
		{
			for (u32 x = 0; x < WIDTH; x++)
			{
				// Generate a world-space ray for this texel
				float2 uv = TexelIndexToUV(x, y, WIDTH, HEIGHT);
				uv.y = 1 - uv.y;																// WAT!?!?
				float3 ray = UVToRay(uv);
				math::vec3f vec_ray = { ray.x, ray.y, ray.z };
				vec_ray = math::qTransformPos(face_rotations_q[i], vec_ray);
				ray.x = vec_ray.x;
				ray.y = vec_ray.y;
				ray.z = vec_ray.z;

				// Evaluate SH along that ray
				float sh_dir[9];
				math::SH_Y2(ray, sh_dir);

				float4 colour;
				colour.x = math::SH_Convolve2(sh_dir, sh_radiance_r);
				colour.y = math::SH_Convolve2(sh_dir, sh_radiance_g);
				colour.z = math::SH_Convolve2(sh_dir, sh_radiance_b);
				colour.w = 1;

				reconstructed_radiance[y * WIDTH + x] = colour;
			}
		}

		// Save reconstruction to disk
		img::Image image(WIDTH, HEIGHT, img::FORMAT_RGBA32, (u8*)reconstructed_radiance, img::DATAOWNER_External);
		core::String256 filename;
		filename.setv("c:/cubemap_sh%d.tga", i);
		m_Resources->images->Save(filename.c_str(), image);
	}
