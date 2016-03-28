#ifndef INPUT_HEADER__
#define INPUT_HEADER__

namespace ul
{
	class Input
	{
	public:
		void Initialize()
		{
			for (int i = 0; i < 256; ++i)
			{
				keys[i] = false;
			}
		}
		void KeyDown(unsigned int keyCode)
		{
			keys[keyCode] = true;
		}
		void KeyUp(unsigned int keyCode)
		{
			keys[keyCode] = false;
		}
		bool IsKeyDown(unsigned int keyCode)
		{
			return keys[keyCode];
		}
	private:
		bool keys[256];
	};
};


#endif