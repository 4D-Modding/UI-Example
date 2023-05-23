//#define DEBUG_CONSOLE // Uncomment this if you want a debug console

// Mod Name. Make sure it matches the mod folder's name
#define MOD_NAME "Custom-UI"
#define MOD_VER "1.0"

#include <Windows.h>
#include <cstdio>
#include <4dm.h>
using namespace fdm;

// thanks mashpoe for giving me this very cool thingy which you need to make some UI things work
void viewportCallbackFunc(void* user, const glm::ivec4& pos, const glm::ivec2& scroll)
{
	GLFWwindow* window = (GLFWwindow*)user;

	// update the render viewport

	int wWidth, wHeight;
	glfwGetWindowSize(window, &wWidth, &wHeight);
	glViewport(pos.x, wHeight - pos.y - pos.w, pos.z, pos.w);

	// create a 2D projection matrix from the specified dimensions and scroll position

	glm::mat4 projection2D = glm::ortho(0.0f, (float)pos.z, (float)pos.w, 0.0f, -1.0f, 1.0f);
	projection2D = glm::translate(projection2D, { scroll.x, scroll.y, 0 });

	// update all 2D shaders

	const Shader* textShader = ShaderManager::get("textShader");
	textShader->use();
	glUniformMatrix4fv(glGetUniformLocation(textShader->ID, "P"), 1, GL_FALSE, &projection2D[0][0]);

	const Shader* tex2DShader = ShaderManager::get("tex2DShader");
	tex2DShader->use();
	glUniformMatrix4fv(glGetUniformLocation(tex2DShader->ID, "P"), 1, GL_FALSE, &projection2D[0][0]);

	const Shader* quadShader = ShaderManager::get("quadShader");
	quadShader->use();
	glUniformMatrix4fv(glGetUniformLocation(quadShader->ID, "P"), 1, GL_FALSE, &projection2D[0][0]);
}

// adding custom Interface to Player
gui::Interface customUIInterface;
gui::Image img;
gui::Image img2;
gui::Text text;

void(__thiscall* Player_init)(Player* self);
void __fastcall Player_init_H(Player* self)
{
	Player_init(self);

	QuadRenderer* qr = new QuadRenderer();
	qr->shader = ShaderManager::get("quadShader");
	qr->init();

	FontRenderer* font = new FontRenderer(ResourceManager::get("pixelFont.png"), ShaderManager::get("textShader"));

	customUIInterface = gui::Interface();
	customUIInterface.font = font;
	customUIInterface.qr = qr;
	customUIInterface.viewportCallback = viewportCallbackFunc;

	text = gui::Text();
	text.setText("some fancy text with some shadow");
	text.size = 1;
	text.alignX(AlignmentX::ALIGN_CENTER_X);
	text.alignY(AlignmentY::ALIGN_CENTER_Y);
	text.offsetX(0);
	text.offsetY(0);

	text.fancy = true;
	text.shadow = true;

	customUIInterface.addElement(&text);


	img = gui::Image();

	img.loadImage("someTex.png", true); // loading custom image from mod folder

	img.alignX(AlignmentX::ALIGN_CENTER_X);
	img.alignY(AlignmentY::ALIGN_BOTTOM);

	img.tr->setColor(1.f, 1.f, 1.f, 0.5f);

	customUIInterface.addElement(&img);

	img2 = gui::Image();

	img2.loadImage("someTex.png", true); // loading custom image from mod folder

	img2.alignX(AlignmentX::ALIGN_CENTER_X);
	img2.alignY(AlignmentY::ALIGN_TOP);

	img2.tr->setColor(1.f, 1.f, 1.f, 0.5f);

	customUIInterface.addElement(&img2);
}
void(__thiscall* Player_renderHud)(Player* self, GLFWwindow* window);
void __fastcall Player_renderHud_H(Player* self, GLFWwindow* window)
{
	Player_renderHud(self, window);

	customUIInterface.window = window;
	customUIInterface.viewportUser = window;

	// render the interface
	glDepthMask(0);
	customUIInterface.render();
	glDepthMask(1);
}

DWORD WINAPI Main_Thread(void* hModule)
{
	// Create console window if DEBUG_CONSOLE is defined
#ifdef DEBUG_CONSOLE
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
#endif
	// Hook to the Player::renderHud function
	Hook(reinterpret_cast<void*>(base + idaOffsetFix(0x7C6A0)), reinterpret_cast<void*>(&Player_renderHud_H), reinterpret_cast<void**>(&Player_renderHud));
	
	// Hook to the Player::init function
	Hook(reinterpret_cast<void*>(base + idaOffsetFix(0x7C400)), reinterpret_cast<void*>(&Player_init_H), reinterpret_cast<void**>(&Player_init));

	EnableHook(0);
	return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD _reason, LPVOID lpReserved)
{
	if (_reason == DLL_PROCESS_ATTACH)
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Main_Thread, hModule, 0, NULL);
	return TRUE;
}
