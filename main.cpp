//#define DEBUG_CONSOLE // Uncomment this if you want a debug console to start. You can use the Console class to print. You can use Console::inStrings to get input.

#include <4dm.h>

using namespace fdm;

// Initialize the DLLMain
initDLL

void viewportCallback(void* user, const glm::ivec4& pos, const glm::ivec2& scroll)
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
	glUniformMatrix4fv(glGetUniformLocation(textShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);

	const Shader* tex2DShader = ShaderManager::get("tex2DShader");
	tex2DShader->use();
	glUniformMatrix4fv(glGetUniformLocation(tex2DShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);

	const Shader* quadShader = ShaderManager::get("quadShader");
	quadShader->use();
	glUniformMatrix4fv(glGetUniformLocation(quadShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);
}

// the custom UI for the Player

TexRenderer texRenderer{};
QuadRenderer qr{};
FontRenderer font{};

gui::Interface ui;
gui::Image img{};
gui::Image img2{};
gui::Text text{};

$hook(void, StateGame, init, StateManager& s)
{
	original(self, s);

	// load the font
	font = { ResourceManager::get("pixelFont.png"), ShaderManager::get("textShader") };

	// initialize the QuadRenderer
	qr.shader = ShaderManager::get("quadShader");
	qr.init();

	// initialize the TexRenderer with the custom texture from our mod.
	texRenderer.texture = ResourceManager::get("someTex.png", true);
	texRenderer.shader = ShaderManager::get("tex2DShader");
	texRenderer.init();

	// initialize the Interface
	ui = gui::Interface{ s.window };
	ui.viewportCallback = viewportCallback;
	ui.viewportUser = s.window;
	ui.font = &font;
	ui.qr = &qr;

	text.setText("some fancy text with some shadow");
	text.size = 1;
	text.alignX(gui::ALIGN_CENTER_X);
	text.alignY(gui::ALIGN_CENTER_Y);
	text.offsetX(0);
	text.offsetY(0);
	text.shadow = true;
	text.fancy = true;

	img.tr = &texRenderer;
	img.width = img.tr->texture->width;
	img.height = img.tr->texture->height;
	img.alignX(gui::ALIGN_CENTER_X);
	img.alignY(gui::ALIGN_BOTTOM);
	img.tr->setColor(1, 1, 1, 0.5f);

	img2.tr = &texRenderer; // the same texture and so the same texture renderer
	img2.width = img.tr->texture->width;
	img2.height = img.tr->texture->height;
	img2.alignX(gui::ALIGN_CENTER_X);
	img2.alignY(gui::ALIGN_TOP);
	img2.tr->setColor(1, 1, 1, 0.5f);

	// add the ui elements
	ui.addElement(&text);
	ui.addElement(&img);
	ui.addElement(&img2);
}

$hook(void, Player, renderHud, GLFWwindow* window)
{
	original(self, window);

	// technically in the game glDepthMask() is used, but for some reason that doesn't work in mods, thus using glDisable/glEnable instead.
	glDisable(GL_DEPTH_TEST);
	ui.render();
	glEnable(GL_DEPTH_TEST);
}

$hook(void, StateIntro, init, StateManager& s)
{
	original(self, s);

	// initialize opengl stuff
	glewExperimental = true;
	glewInit();
	glfwInit();
}
