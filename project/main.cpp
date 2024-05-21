
#ifdef _WIN32
extern "C" _declspec(dllexport) unsigned int NvOptimusEnablement = 0x00000001;
#endif

#include <GL/glew.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <stb_image.h>

#include <labhelper.h>
#include <imgui.h>

#include <perf.h>

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include <Model.h>
#include "hdr.h"
#include "fbo.h"

#include "Grid.h"
#include "noise.h"
#include "Water.h"


///////////////////////////////////////////////////////////////////////////////
// Various globals
///////////////////////////////////////////////////////////////////////////////
SDL_Window* g_window = nullptr;
float currentTime = 0.0f;
float previousTime = 0.0f;
float deltaTime = 0.0f;
int windowWidth, windowHeight;

// Mouse input
ivec2 g_prevMouseCoords = { -1, -1 };
bool g_isMouseDragging = false;

///////////////////////////////////////////////////////////////////////////////
// Shader programs
///////////////////////////////////////////////////////////////////////////////
GLuint simpleShaderProgram;
GLuint testShader;
GLuint waterShader;



///////////////////////////////////////////////////////////////////////////////
// Light source
///////////////////////////////////////////////////////////////////////////////
vec3 lightPosition = vec3(0.0f, 813.0f, 752.0f);
float sunangle = 0.0f;
vec3 lightDirection;

///////////////////////////////////////////////////////////////////////////////
// Camera parameters.
///////////////////////////////////////////////////////////////////////////////
vec3 cameraPosition(186.0f, 829.0f, 1080.0f);
vec3 cameraDirection = normalize(vec3(0.0f) - cameraPosition);

// Camera speed
float cameraSpeedBase = 100.f;
float cameraSpeed = cameraSpeedBase;
float shiftSpeed = 10.0;

vec3 worldUp(0.0f, 1.0f, 0.0f);

///////////////////////////////////////////////////////////////////////////////
// Models
///////////////////////////////////////////////////////////////////////////////
labhelper::Model* sphereModel = nullptr;

mat4 gridMatrix;

void loadShaders(bool is_reload)
{
	GLuint shader = labhelper::loadShaderProgram("../project/simple.vert", "../project/simple.frag", is_reload);
	if(shader != 0)
	{
		simpleShaderProgram = shader;
	}
	
	shader = labhelper::loadShaderProgram("../project/testShader.vert", "../project/testShader.frag", is_reload);
	if (shader != 0)
	{
		testShader = shader;
	}

	shader = labhelper::loadShaderProgram("../project/waterShader.vert", "../project/waterShader.frag", is_reload);
	if (shader != 0)
	{
		waterShader = shader;
	}
}


///////////////////////////////////////////////////////////////////////////////
/// This function is called once at the start of the program and never again
///////////////////////////////////////////////////////////////////////////////


void initialize()
{
	ENSURE_INITIALIZE_ONLY_ONCE();

	///////////////////////////////////////////////////////////////////////
	//		Load Shaders
	///////////////////////////////////////////////////////////////////////
	loadShaders(false);



	///////////////////////////////////////////////////////////////////////
	// Load models and set up model matrices
	///////////////////////////////////////////////////////////////////////

	sphereModel = labhelper::loadModelFromOBJ("../scenes/sphere.obj");


	glEnable(GL_DEPTH_TEST); // enable Z-buffering
	glEnable(GL_CULL_FACE);  // enables backface culling
}

void debugDrawLight(const glm::mat4& viewMatrix,
                    const glm::mat4& projectionMatrix,
                    const glm::vec3& worldSpaceLightPos)
{
	mat4 modelMatrix = glm::translate(worldSpaceLightPos);
	modelMatrix = glm::scale(modelMatrix, vec3(30.0f));
	glUseProgram(simpleShaderProgram);
	labhelper::setUniformSlow(simpleShaderProgram, "material_color", vec3(1.0f, 1.0f, 0.0f));

	labhelper::setUniformSlow(simpleShaderProgram, "modelViewProjectionMatrix",
	                          projectionMatrix * viewMatrix * modelMatrix);
	labhelper::render(sphereModel);
}




///////////////////////////////////////////////////////////////////////////////
/// This function will be called once per frame, so the code to set up
/// the scene for rendering should go here
///////////////////////////////////////////////////////////////////////////////
void display(void)
{
	labhelper::perf::Scope s( "Display" );

	///////////////////////////////////////////////////////////////////////////
	// Check if window size has changed and resize buffers as needed
	///////////////////////////////////////////////////////////////////////////
	{
		int w, h;
		SDL_GetWindowSize(g_window, &w, &h);
		if(w != windowWidth || h != windowHeight)
		{
			windowWidth = w;
			windowHeight = h;
		}
	}


	///////////////////////////////////////////////////////////////////////////
	// Draw from camera
	///////////////////////////////////////////////////////////////////////////
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

}


///////////////////////////////////////////////////////////////////////////////
/// This function is used to update the scene according to user input
///////////////////////////////////////////////////////////////////////////////
bool handleEvents(void)
{
	// check events (keyboard among other)
	SDL_Event event;
	bool quitEvent = false;
	while(SDL_PollEvent(&event))
	{
		labhelper::processEvent( &event );

		if(event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE))
		{
			quitEvent = true;
		}
		if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_g)
		{
			if ( labhelper::isGUIvisible() )
			{
				labhelper::hideGUI();
			}
			else
			{
				labhelper::showGUI();
			}
		}
		if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_1)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		}
		if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_2)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_3)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT
		   && (!labhelper::isGUIvisible() || !ImGui::GetIO().WantCaptureMouse))
		{
			g_isMouseDragging = true;
			int x;
			int y;
			SDL_GetMouseState(&x, &y);
			g_prevMouseCoords.x = x;
			g_prevMouseCoords.y = y;
		}

		if(!(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)))
		{
			g_isMouseDragging = false;
		}

		if(event.type == SDL_MOUSEMOTION && g_isMouseDragging)
		{
			// More info at https://wiki.libsdl.org/SDL_MouseMotionEvent
			int delta_x = event.motion.x - g_prevMouseCoords.x;
			int delta_y = event.motion.y - g_prevMouseCoords.y;
			float rotationSpeed = 0.1f;
			mat4 yaw = rotate(rotationSpeed * deltaTime * -delta_x, worldUp);
			mat4 pitch = rotate(rotationSpeed * deltaTime * -delta_y,
			                    normalize(cross(cameraDirection, worldUp)));
			cameraDirection = vec3(pitch * yaw * vec4(cameraDirection, 0.0f));
			g_prevMouseCoords.x = event.motion.x;
			g_prevMouseCoords.y = event.motion.y;
		}
	}

	// check keyboard state (which keys are still pressed)
	const uint8_t* state = SDL_GetKeyboardState(nullptr);
	vec3 cameraRight = cross(cameraDirection, worldUp);

	if(state[SDL_SCANCODE_W])
	{
		cameraPosition += cameraSpeed * deltaTime * cameraDirection;
	}
	if(state[SDL_SCANCODE_S])
	{
		cameraPosition -= cameraSpeed * deltaTime * cameraDirection;
	}
	if(state[SDL_SCANCODE_A])
	{
		cameraPosition -= cameraSpeed * deltaTime * cameraRight;
	}
	if(state[SDL_SCANCODE_D])
	{
		cameraPosition += cameraSpeed * deltaTime * cameraRight;
	}
	if(state[SDL_SCANCODE_Q]) // SDL_SCANCODE_LCTRL
	{
		cameraPosition -= cameraSpeed * deltaTime * worldUp;
	}
	if(state[SDL_SCANCODE_E]) // SDL_SCANCODE_SPACE
	{
		cameraPosition += cameraSpeed * deltaTime * worldUp;
	}
	if (state[SDL_SCANCODE_RIGHT])
	{
		lightPosition += cameraSpeed * deltaTime * vec3(1.0f, 0.0, 0.0) / 2.0f;
	}
	if (state[SDL_SCANCODE_LEFT])
	{
		lightPosition -= cameraSpeed * deltaTime * vec3(1.0f, 0.0, 0.0) / 2.0f;
	}
	if (state[SDL_SCANCODE_LSHIFT] == true)
	{
		cameraSpeed = cameraSpeedBase * shiftSpeed;
	}
	if (state[SDL_SCANCODE_LSHIFT] == false)
	{
		cameraSpeed = cameraSpeedBase;
	}
	return quitEvent;
}




// Time of day t ranges from 0 (sunrise) to 1 (sunset)
glm::vec3 rotateSun(float t)
{
	float degrees = glm::mix(-90.0f, 90.0f, t); // Interpolate angle from -90 to 90
	glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), glm::vec3(0, 0, 1)); // Rotate around Z-axis
	glm::vec4 initialDir = glm::vec4(1, 0, 0, 1); // Starting east
	glm::vec4 rotatedDir = rot * initialDir;
	return glm::vec3(rotatedDir);
}

///////////////////////////////////////////////////////////////////////////////
/// This function is to hold the general GUI logic
///////////////////////////////////////////////////////////////////////////////
void gui()
{
	// ----------------- Set variables --------------------------



	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
	            ImGui::GetIO().Framerate);
	ImGui::SliderFloat("Light pos x", &lightPosition.x, -10000.0f, 10000.0f);
	ImGui::SliderFloat("Light pos y", &lightPosition.y, -10000.0f, 10000.0f);
	ImGui::SliderFloat("Light pos z", &lightPosition.z, -10000.0f, 10000.0f);
	ImGui::Text("Camera Pos:  x: %.3f  y: %.3f  z: %.3f", cameraPosition.x, cameraPosition.y, cameraPosition.z);
	ImGui::SliderFloat("Sun angle", &sunangle, 0.0f, 2.0f);
	ImGui::Text("Light Direction:  x: %.3f  y: %.3f  z: %.3f", lightDirection.x, lightDirection.y, lightDirection.z);


	// ----------------------------------------------------------


	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	labhelper::perf::drawEventsWindow();
}

int main(int argc, char* argv[])
{
	g_window = labhelper::init_window_SDL("OpenGL Project");


	initialize();
	int w, h;
	SDL_GetWindowSize(g_window, &w, &h);
	if (w != windowWidth || h != windowHeight)
	{
		windowWidth = w;
		windowHeight = h;
	}
	bool stopRendering = false;

	auto startTime = std::chrono::system_clock::now();																							// Start clock, create inition chunks
	auto start = std::chrono::high_resolution_clock::now();

	// Generate initial grid parameters
	int gridWidth = 240;
	int gridHeight = 240;
	int xStartPos;
	int yStartPos;
	float cellSize = 10.0;
	float perlinScale = 1500.0;
	float voronoiScale = 200.0;

	float LoD = 1.0; // Tillfällig

	//GridChunk initialChunk1;
	GridChunk initialChunk2;
	GridChunk initialChunk3;
																																				// *** KOMMENTARER: Lägg till "levels of detail". Går ej att stoppa in negativa koordinater just nu. Kanske borde byta från (x1,y1,x2,y2) till (x1,x2,y1,y2)? Fixa "GridChunk::generateChunkGrids". Gör klart "GridChunk::gridChunkCenter()"
	// createNewStandardChunk(xChunkStart, yChunkStart, xChunkEnd, yChunkEnd);																	// Standard (värden i grid.cpp)
	//initialChunk1.createNewStandardChunk(0, 0, 1, 1, -500);
	// createNewChunk(xChunkStart, yChunkStart, xChunkEnd, yChunkEnd, gridWidth, gridHeight, cellSize, perlinScale, voronoiScale);				// Välj variabler
	initialChunk2.createNewChunk(0, 0, 1, 1, gridWidth / LoD, gridHeight / LoD, cellSize * LoD, 900, perlinScale, voronoiScale);				// Artificiellt lägre LoD. Måste ändra på filter-variablerna också för att det ska bli korrekt?

	// Ocean
	initialChunk3.createNewChunk(0, 0, 1, 1, 2, 2, 2400, 50, 0, 0);

	gridMatrix = mat4(1.0f);

	gridMatrix = glm::rotate(gridMatrix, glm::radians(90.0f), glm::vec3(1, 0, 0));
	gridMatrix = glm::translate(gridMatrix, glm::vec3(-gridWidth / 2.0f, -gridHeight / 2.0f, 0.0f));

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::duration<float>>(stop - start);
	std::cout << "Initial grid(s); generation time: " << duration.count() << std::endl;															// End clock, create inition chunks


	
	// SETUP
	// DESSA MÅSTE UPPDATERAS NÄR MAN ÄNDRAR STORLEKEN PÅ SKÄRMEN!!!
	int width = 1280;
	int height = 720;

	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
	};
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<float> normals;

	// PLANE
	float waterPlane[] = {
	2400, 2400, 0.0f,		1.0f, 1.0f,
	2400, 0, 0.0f,			0.0f, 1.0f,
	0, 2400, 0.0f,			1.0f, 0.0f,

	2400, 0, 0.0f,			0.0f, 1.0f,
	0, 0, 0.0f,				0.0f, 0.0f,
	0, 2400, 0.0f,			1.0f, 0.0f
};

	unsigned int waterPlaneVAO, waterPlaneVBO;
	glGenVertexArrays(1, &waterPlaneVAO);
	glGenBuffers(1, &waterPlaneVBO);
	glBindVertexArray(waterPlaneVAO);
	glBindBuffer(GL_ARRAY_BUFFER, waterPlaneVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(waterPlane), &waterPlane, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	// FRAMEBUFFERS
	// REFLECTION
	unsigned int reflectionFBO;
	glGenFramebuffers(1, &reflectionFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);

	unsigned int reflectionRBO;
	glGenRenderbuffers(1, &reflectionRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, reflectionRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, reflectionRBO);

	unsigned int reflectionTextureColorbuffer;
	glGenTextures(1, &reflectionTextureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, reflectionTextureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectionTextureColorbuffer, 0);

	// REFRACTION
	unsigned int refractionFBO;
	glGenFramebuffers(1, &refractionFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);

	unsigned int refractionRBO;
	glGenRenderbuffers(1, &refractionRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, refractionRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, refractionRBO);

	unsigned int refractionTextureColorbuffer;
	glGenTextures(1, &refractionTextureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, refractionTextureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refractionTextureColorbuffer, 0);

	unsigned int refractionTextureDepthbuffer;
	glGenTextures(1, &refractionTextureDepthbuffer);
	glBindTexture(GL_TEXTURE_2D, refractionTextureDepthbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, windowWidth, windowHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, refractionTextureDepthbuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, refractionTextureDepthbuffer, 0);

	// Load DUDVmap
	unsigned int DUDVtexture;
	glGenTextures(1, &DUDVtexture);
	glBindTexture(GL_TEXTURE_2D, DUDVtexture);

	int DUDVwidth, DUDVheight, DUDVnrChannels;
	unsigned char *DUDVdata = stbi_load("../textures/water/waterDUDV.png", &DUDVwidth, &DUDVheight, &DUDVnrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, DUDVwidth, DUDVheight, 0, GL_RGB, GL_UNSIGNED_BYTE, DUDVdata);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Load Normalmap
	unsigned int Normaltexture;
	glGenTextures(1, &Normaltexture);
	glBindTexture(GL_TEXTURE_2D, Normaltexture);

	int Normalwidth, Normalheight, NormalnrChannels;
	unsigned char* Normaldata = stbi_load("../textures/water/waterNormal.png", &Normalwidth, &Normalheight, &NormalnrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Normalwidth, Normalheight, 0, GL_RGB, GL_UNSIGNED_BYTE, Normaldata);
	glGenerateMipmap(GL_TEXTURE_2D);

	// VARIABLES
	float waterHeight = 0.0f;
	float waveSpeed = 0.00025f;
	float waveScale = 7.0f;

	// END
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Start render-loop
	while(!stopRendering)
	{
		{
			int w, h;
			SDL_GetWindowSize(g_window, &w, &h);
			if (w != windowWidth || h != windowHeight)
			{
				windowWidth = w;
				windowHeight = h;
			}
		}


		//Update screen size
		glBindRenderbuffer(GL_RENDERBUFFER, reflectionRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glBindTexture(GL_TEXTURE_2D, refractionTextureDepthbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, windowWidth, windowHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

		glBindRenderbuffer(GL_RENDERBUFFER, refractionRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glBindTexture(GL_TEXTURE_2D, refractionTextureColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, refractionTextureDepthbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, windowWidth, windowHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		// *** SETUP ***
		//update currentTime
		std::chrono::duration<float> timeSinceStart = std::chrono::system_clock::now() - startTime;
		previousTime = currentTime;
		currentTime = timeSinceStart.count();
		deltaTime = currentTime - previousTime;
		// check events (keyboard among other)
		stopRendering = handleEvents();
		// Inform imgui of new frame
		labhelper::newFrame( g_window );
		// render to window
		display();
		lightDirection = rotateSun(sunangle);
		mat4 projMatrix;
		mat4 viewMatrix;

		// *** LOAD DEFAULT VALUES TO SHADERS ***
		projMatrix = perspective(radians(45.0f), float(windowWidth) / float(windowHeight), 0.1f, 2000000.0f);
		viewMatrix = lookAt(cameraPosition, cameraPosition + cameraDirection, worldUp);
		
		// test shader
		glUseProgram(testShader);
		labhelper::setUniformSlow(testShader, "viewPos", cameraPosition);
		labhelper::setUniformSlow(testShader, "lightDirection", lightDirection);
		labhelper::setUniformSlow(testShader, "lightDiffuse", vec3(1.0f));
		labhelper::setUniformSlow(testShader, "lightAmbient", vec3(0.1f));
		labhelper::setUniformSlow(testShader, "lightSpecular", vec3(1.0f));
		labhelper::setUniformSlow(testShader, "materialShininess", (1.0f));

		labhelper::setUniformSlow(testShader, "materialColor1", vec3(0.0f, 1.0f, 0.0f)); // Gräs
		labhelper::setUniformSlow(testShader, "materialColor2", vec3(0.0f, 0.0f, 1.0f)); // Vatten
		labhelper::setUniformSlow(testShader, "materialColor3", vec3(0.5f, 0.5f, 0.5f)); // Lutning
		labhelper::setUniformSlow(testShader, "materialColor4", vec3(0.7f, 0.7f, 0.5f)); // Sand
		labhelper::setUniformSlow(testShader, "materialColor5", vec3(1.0f, 1.0f, 1.0f)); // Snö

		labhelper::setUniformSlow(testShader, "model", gridMatrix);
		labhelper::setUniformSlow(testShader, "waterPlaneHeight", waterHeight); // Height if the water plane (default 0.0)
		labhelper::setUniformSlow(testShader, "waterPlaneDirection", 1.0f); // 1 = cull > height, -1 = cull < height

		// water shader
		glUseProgram(waterShader);
		labhelper::setUniformSlow(waterShader, "projection", projMatrix);
		labhelper::setUniformSlow(waterShader, "view", viewMatrix);
		labhelper::setUniformSlow(waterShader, "model", gridMatrix);
		labhelper::setUniformSlow(waterShader, "reflectionTexture", 0);
		labhelper::setUniformSlow(waterShader, "refractionTexture", 1);
		labhelper::setUniformSlow(waterShader, "dudvMap", 2);
		labhelper::setUniformSlow(waterShader, "waveScale", waveScale);

		labhelper::setUniformSlow(waterShader, "viewPos", cameraPosition);
		labhelper::setUniformSlow(waterShader, "lightDirection", lightDirection);
		labhelper::setUniformSlow(waterShader, "lightDiffuse", vec3(1.0f));
		labhelper::setUniformSlow(waterShader, "lightAmbient", vec3(0.1f));
		labhelper::setUniformSlow(waterShader, "lightSpecular", vec3(1.0f));
		labhelper::setUniformSlow(waterShader, "materialShininess", (1.0f));

		// *** RENDER ***
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CLIP_DISTANCE0);

		// wave movement
		waveScale += waveSpeed;
		//waveScale = fmod(waveScale, 10); // "hackar" ibland vid (waveScale = 1)
		if (waveScale >= 8.0) {
			waveScale -= 1.0;
		}
		labhelper::setUniformSlow(waterShader, "waveScale", waveScale);

		// ground, reflection (water)
		glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glUseProgram(testShader);
		labhelper::setUniformSlow(testShader, "waterPlaneDirection", -1.0f);

		// Change camera for the reflection
		float cameraDistance = 2 * (cameraPosition.y - waterHeight);

		cameraPosition = {cameraPosition.x, cameraPosition.y - cameraDistance, cameraPosition.z};
		cameraDirection.y = -cameraDirection.y;
		labhelper::setUniformSlow(testShader, "viewPos", cameraPosition);
		projMatrix = perspective(radians(45.0f), float(windowWidth) / float(windowHeight), 0.1f, 2000000.0f); // 0.1f, 2000000.0f near/far plane
		viewMatrix = lookAt(cameraPosition, cameraPosition + cameraDirection, worldUp);
		labhelper::setUniformSlow(testShader, "projection", projMatrix);
		labhelper::setUniformSlow(testShader, "view", viewMatrix);

		initialChunk2.DrawGridChunk();

		cameraPosition = { cameraPosition.x, cameraPosition.y + cameraDistance, cameraPosition.z};
		cameraDirection.y = -cameraDirection.y;
		labhelper::setUniformSlow(testShader, "viewPos", cameraPosition);
		projMatrix = perspective(radians(45.0f), float(windowWidth) / float(windowHeight), 0.1f, 2000000.0f);
		viewMatrix = lookAt(cameraPosition, cameraPosition + cameraDirection, worldUp);
		labhelper::setUniformSlow(testShader, "projection", projMatrix);
		labhelper::setUniformSlow(testShader, "view", viewMatrix);
		
		// ground, refraction (water)
		glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(testShader);
		labhelper::setUniformSlow(testShader, "waterPlaneDirection", 1.0f);
		initialChunk2.DrawGridChunk();


		// Draw scene
		
		// ground
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glUseProgram(testShader);
		labhelper::setUniformSlow(testShader, "waterPlaneDirection", 0.0f);
		initialChunk2.DrawGridChunk();

		
		// water plane
		glUseProgram(waterShader);
		glBindVertexArray(waterPlaneVAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, reflectionTextureColorbuffer);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, refractionTextureColorbuffer);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, DUDVtexture);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, Normaltexture);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, refractionTextureDepthbuffer);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisable(GL_BLEND);
		

		// *** GUI
		// Render overlay GUI.
		gui();

		// Finish the frame and render the GUI
		labhelper::finishFrame();

		// Swap front and back buffer. This frame will now been displayed.
		SDL_GL_SwapWindow(g_window);
	}
	// Free Models
	labhelper::freeModel(sphereModel);

	// Shut down everything. This includes the window and all other subsystems.
	labhelper::shutDown(g_window);
	return 0;
}
