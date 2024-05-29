
#ifdef _WIN32
extern "C" _declspec(dllexport) unsigned int NvOptimusEnablement = 0x00000001;
#endif

#include <GL/glew.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <iostream>

#include <labhelper.h>
#include <imgui.h>

#include <perf.h>

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
//using namespace glm;

#include <Model.h>
#include "hdr.h"
#include "fbo.h"

#include "Grid.h"
#include "noise.h"
#include "myHelper/shader_c.h"
#include "myHelper/shader_s.h"





#include <stb_image.h>


///////////////////////////////////////////////////////////////////////////////
// Various globals
///////////////////////////////////////////////////////////////////////////////
SDL_Window* g_window = nullptr;
float currentTime = 0.0f;
float previousTime = 0.0f;
float deltaTime = 0.0f;
int windowWidth, windowHeight;

// Mouse input
glm::ivec2 g_prevMouseCoords = { -1, -1 };
bool g_isMouseDragging = false;

///////////////////////////////////////////////////////////////////////////////
// Shader programs
///////////////////////////////////////////////////////////////////////////////
GLuint simpleShaderProgram;
GLuint testShader;
GLuint quadShader;
GLuint waterShader;
GLuint skyShader;




///////////////////////////////////////////////////////////////////////////////
// Light source
///////////////////////////////////////////////////////////////////////////////
glm::vec3 lightPosition = glm::vec3(0.0f, 813.0f, 752.0f);
float sunangle = 0.6f;
glm::vec3 lightDirection;

///////////////////////////////////////////////////////////////////////////////
// Camera parameters.
///////////////////////////////////////////////////////////////////////////////
glm::vec3 cameraPosition(-0.1f + 12000.0f, 130.0f, -0.1f + 12000.0f);
glm::vec3 cameraDirection = glm::normalize(glm::vec3(199.0f, 80.0f, 327.0f) - cameraPosition);

float farPlane = 200000.0f;
float nearPlane = 100.0f;

// Camera speed
float cameraSpeedBase = 100.f;
float cameraSpeed = cameraSpeedBase;
float shiftSpeed = 10.0;

glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

///////////////////////////////////////////////////////////////////////////////
// Models
///////////////////////////////////////////////////////////////////////////////
labhelper::Model* sphereModel = nullptr;

glm::mat4 gridMatrix;
int gridSize = 3000;
float noiseScale = 0.01;
float simpexScale = 2964;
float worleyScale = 500;
float simpexAmplitude = 1305;
float worleyAmplitude = 100;
float ratio = 1.0;


float gridZ = -1500.0f;
glm::vec3 gridSpeed;



bool followGrid = false;
bool backFaceCulling = false;

glm::vec3 vertexFollow;
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
	shader = labhelper::loadShaderProgram("../project/background.vert", "../project/background.frag", is_reload);
	if (shader != 0)
	{
		quadShader = shader;
	}
	shader = labhelper::loadShaderProgram("../project/waterShader.vert", "../project/waterShader.frag", is_reload);
	if (shader != 0)
	{
		waterShader = shader;
	}
	shader = labhelper::loadShaderProgram("../project/skyShader.vert", "../project/skyShader.frag", is_reload);
	if (shader != 0)
	{
		skyShader = shader;
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
	//glEnable(GL_CULL_FACE);  // enables backface culling
}

void debugDrawLight(const glm::mat4& viewMatrix,
                    const glm::mat4& projectionMatrix,
                    const glm::vec3& worldSpaceLightPos)
{
	glm::mat4 modelMatrix = glm::translate(worldSpaceLightPos);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(30.0f));
	glUseProgram(simpleShaderProgram);
	labhelper::setUniformSlow(simpleShaderProgram, "material_color", glm::vec3(1.0f, 1.0f, 0.0f));

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
			glm::mat4 yaw = rotate(rotationSpeed * deltaTime * -delta_x, worldUp);
			glm::mat4 pitch = rotate(rotationSpeed * deltaTime * -delta_y,
			                    normalize(cross(cameraDirection, worldUp)));
			cameraDirection = glm::vec3(pitch * yaw * glm::vec4(cameraDirection, 0.0f));
			g_prevMouseCoords.x = event.motion.x;
			g_prevMouseCoords.y = event.motion.y;
		}
	}

	// check keyboard state (which keys are still pressed)
	const uint8_t* state = SDL_GetKeyboardState(nullptr);
	glm::vec3 cameraRight = cross(cameraDirection, worldUp);
	/*
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
	*/
	if(state[SDL_SCANCODE_Q])
	{
		cameraPosition -= cameraSpeed * deltaTime * worldUp;
	}
	if(state[SDL_SCANCODE_E])
	{
		cameraPosition += cameraSpeed * deltaTime * worldUp;
	}
	
	if ((state[SDL_SCANCODE_A] || state[SDL_SCANCODE_D] ||
		state[SDL_SCANCODE_W] || state[SDL_SCANCODE_S] ) && !followGrid)
	{
		if (state[SDL_SCANCODE_A])
		{
			//gridSpeed = -glm::vec3(cameraRight.x, cameraRight.z, 0) * (cameraSpeed/100.0f);
			cameraPosition -= cameraSpeed * deltaTime * cameraRight;
	
		}
		else if (state[SDL_SCANCODE_D])
		{
			//gridSpeed = glm::vec3(cameraRight.x, cameraRight.z, 0) * (cameraSpeed / 100.0f);
			cameraPosition += cameraSpeed * deltaTime * cameraRight;
		}
		else if (state[SDL_SCANCODE_W])
		{
			//gridSpeed = glm::vec3(cameraDirection.x, cameraDirection.z, 0) * (cameraSpeed / 100.0f);
			cameraPosition += cameraSpeed * deltaTime * cameraDirection;
		}
		else if (state[SDL_SCANCODE_S])
		{
			//gridSpeed = -glm::vec3(cameraDirection.x, cameraDirection.z, 0) * (cameraSpeed / 100.0f);
			cameraPosition -= cameraSpeed * deltaTime * cameraDirection;
		}
	}
	else
	{
		gridSpeed = glm::vec3(0.0);
	}
	
	
	if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_LEFT] ||
		state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_DOWN])
	{
		if (state[SDL_SCANCODE_RIGHT])
		{
			gridSpeed.y = 1.0;
		}
		if (state[SDL_SCANCODE_LEFT])
		{
			gridSpeed.y = -1.0;
		}
		if (state[SDL_SCANCODE_UP])
		{
			gridSpeed.x = 1.0;
		}
		if (state[SDL_SCANCODE_DOWN])
		{
			gridSpeed.x = -1.0;
		}
	}
	else
	{
		gridSpeed = glm::vec3(0.0);
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
	ImGui::Text("Camera Dir:  x: %.3f  y: %.3f  z: %.3f", cameraDirection.x, cameraDirection.y, cameraDirection.z);
	ImGui::Text("Grid Pos:  x: %.3f  y: %.3f  z: %.3f", gridSpeed.x, gridSpeed.y, gridSpeed.z);
	ImGui::Text("Vertex Follow:  x: %.3f  y: %.3f  z: %.3f", vertexFollow.x, vertexFollow.y, vertexFollow.z);
	ImGui::SliderFloat("Sun angle", &sunangle, 0.0f, 2.0f);
	ImGui::Text("Light Direction:  x: %.3f  y: %.3f  z: %.3f", lightDirection.x, lightDirection.y, lightDirection.z);
	ImGui::SliderFloat("simpexScale", &simpexScale, 0.1f, 3000.0f);
	ImGui::SliderFloat("worleyScale", &worleyScale, 0.1f, 3000.0f);
	ImGui::SliderFloat("simpexAmplitude", &simpexAmplitude, 0.1f, 3000.0f);
	ImGui::SliderFloat("worleyAmplitude", &worleyAmplitude, 0.1f, 3000.0f);
	ImGui::SliderFloat("ratio", &ratio, 0.0f, 1.0f);
	ImGui::SliderInt("Grid Size", &gridSize, 10, 10000);
	ImGui::SliderFloat("Far plane", &farPlane, 100.0f, 2000000.0f);
	ImGui::SliderFloat("Near Plane", &nearPlane, 0.1f, 10000.0f);
	ImGui::Checkbox("Follow", &followGrid);
	ImGui::Checkbox("Back face culling", &backFaceCulling);



	// ----------------------------------------------------------


	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	labhelper::perf::drawEventsWindow();
}

void computeShaderSetupQuery()
{
	// query limitations
	// -----------------
	int max_compute_work_group_count[3];
	int max_compute_work_group_size[3];
	int max_compute_work_group_invocations;

	for (int idx = 0; idx < 3; idx++) {
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &max_compute_work_group_count[idx]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, idx, &max_compute_work_group_size[idx]);
	}
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_compute_work_group_invocations);

	std::cout << "OpenGL Limitations: " << std::endl;
	std::cout << "maximum number of work groups in X dimension " << max_compute_work_group_count[0] << std::endl;
	std::cout << "maximum number of work groups in Y dimension " << max_compute_work_group_count[1] << std::endl;
	std::cout << "maximum number of work groups in Z dimension " << max_compute_work_group_count[2] << std::endl;

	std::cout << "maximum size of a work group in X dimension " << max_compute_work_group_size[0] << std::endl;
	std::cout << "maximum size of a work group in Y dimension " << max_compute_work_group_size[1] << std::endl;
	std::cout << "maximum size of a work group in Z dimension " << max_compute_work_group_size[2] << std::endl;

	std::cout << "Number of invocations in a single local work group that may be dispatched to a compute shader " << max_compute_work_group_invocations << std::endl;
}



struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
};

unsigned int GrassTexture;
unsigned int StoneTexture;
unsigned int SandTexture;
unsigned int SnowTexture;

void loadTextures()
{
	// Load "Grass"

	glGenTextures(1, &GrassTexture);
	glBindTexture(GL_TEXTURE_2D, GrassTexture);

	int GrassWidth, GrassHeight, GrassNrChannels;
	unsigned char* GrassData = stbi_load("../textures/land/testGrass_small.png", &GrassWidth, &GrassHeight, &GrassNrChannels, 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GrassWidth, GrassHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, GrassData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Load "Stone"

	glGenTextures(1, &StoneTexture);
	glBindTexture(GL_TEXTURE_2D, StoneTexture);

	int StoneWidth, StoneHeight, StoneNrChannels;
	unsigned char* StoneData = stbi_load("../textures/land/testStone_small.png", &StoneWidth, &StoneHeight, &StoneNrChannels, 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, StoneWidth, StoneHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, StoneData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Load "Sand"

	glGenTextures(1, &SandTexture);
	glBindTexture(GL_TEXTURE_2D, SandTexture);

	int SandWidth, SandHeight, SandNrChannels;
	unsigned char* SandData = stbi_load("../textures/land/testSand_small.png", &SandWidth, &SandHeight, &SandNrChannels, 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SandWidth, SandHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, SandData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Load "Snow"

	glGenTextures(1, &SnowTexture);
	glBindTexture(GL_TEXTURE_2D, SnowTexture);

	int SnowWidth, SnowHeight, SnowNrChannels;
	unsigned char* SnowData = stbi_load("../textures/land/testSnow_small.png", &SnowWidth, &SnowHeight, &SnowNrChannels, 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SnowWidth, SnowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, SnowData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);
}


int main(int argc, char* argv[])
{
	

	g_window = labhelper::init_window_SDL("OpenGL Project");

	
	initialize();

	// Build and compile shaders
	ComputeShader computeShader("../project/simpleComputeShader.glsl");
	// query limitations
	computeShaderSetupQuery();

	bool stopRendering = false;

	
	// Debug time. Put between start and stop
	auto startTime = std::chrono::system_clock::now();																				// Start clock, create inition chunks
	auto start = std::chrono::high_resolution_clock::now();		
	Grid grid;
	grid.generateGrid();
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::duration<float>>(stop - start);
	std::cout << "Initial grid(s); generation time: " << duration.count() << std::endl;
	


	// Debug
	/*
	glBindBuffer(GL_ARRAY_BUFFER, grid.getVBO());
	Vertex* mappedVertices = (Vertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	for (int i = 0; i < 10; ++i) {
		printf("Vertex %d: Pos(%f, %f, %f), Norm(%f, %f, %f)\n",
			i,
			mappedVertices[i].position.x, mappedVertices[i].position.y, mappedVertices[i].position.z,
			mappedVertices[i].normal.x, mappedVertices[i].normal.y, mappedVertices[i].normal.z);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	*/

	gridMatrix = glm::mat4(1.0f);
	gridSpeed = glm::vec3(0.0f, 0.0f, 0.0f); // Grid position
	glm::mat4 initialRotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(90.0f, 0.0f, 0.0f)); // Example rotation
	gridMatrix = initialRotation; // Applying initial rotation


											// End clock, create inition chunks



	// Buffer for vertexFollow
	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3), NULL, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	// Textures
	loadTextures();


	// Create water plane
		// PLANE
	float waterPlane[] = {
	24000, 24000, 200.0f,		1.0f, 1.0f,
	24000, 0, 200.0f,			0.0f, 1.0f,
	0, 24000, 200.0f,			1.0f, 0.0f,

	24000, 0, 200.0f,			0.0f, 1.0f,
	0, 0, 200.0f,				0.0f, 0.0f,
	0, 24000, 200.0f,			1.0f, 0.0f
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
	glBindVertexArray(0);


	// Reflection: Frame and renderbuffer
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
	unsigned char* DUDVdata = stbi_load("../textures/water/waterDUDV.png", &DUDVwidth, &DUDVheight, &DUDVnrChannels, 0);

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
	



	// Generate and bind the framebuffer
	unsigned int skyFBO;
	glGenFramebuffers(1, &skyFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, skyFBO);

	unsigned int skyRBO;
	glGenRenderbuffers(1, &skyRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, skyRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, skyRBO);

	unsigned int skyTexture;
	glGenTextures(1, &skyTexture);
	glBindTexture(GL_TEXTURE_2D, skyTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, skyTexture, 0);
	

	// Check for framebuffer completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << glGetError() << " Framebuffer incomplete: ";
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch (status) {
		case GL_FRAMEBUFFER_UNDEFINED:
			std::cout << "GL_FRAMEBUFFER_UNDEFINED" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" << std::endl;
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			std::cout << "GL_FRAMEBUFFER_UNSUPPORTED" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS" << std::endl;
			break;
		default:
			std::cout << "Unknown error" << std::endl;
			break;
		}
	}
	else {
		std::cout << "Framebuffer complete" << std::endl;
	}

	// END
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	
	// Start render-loop
	while (!stopRendering)
	{

		//position.x = cameraPosition.x;
		// Combine the initial rotation with the new position
		//gridMatrix = initialRotation; // Reset to initial rotation
		//gridMatrix = glm::translate(gridMatrix, position); // Apply new translation
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glm::vec3* ptr = (glm::vec3*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec3), GL_MAP_READ_BIT);

		if (ptr) {
			vertexFollow = *ptr;
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	
		if (followGrid) {
			cameraPosition = glm::vec3(vertexFollow.x + 12000.0f, cameraPosition.y, vertexFollow.y + 12000.0f);
		}
		if (backFaceCulling)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}
		
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
		glm::mat4 projMatrix = glm::perspective(glm::radians(45.0f), float(windowWidth) / float(windowHeight), nearPlane, farPlane);
		glm::mat4 viewMatrix = lookAt(cameraPosition, cameraPosition + cameraDirection, worldUp);

		// ground, reflection (water)
		glBindFramebuffer(GL_FRAMEBUFFER, skyFBO);
		glBindTexture(GL_TEXTURE_2D, skyTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindRenderbuffer(GL_RENDERBUFFER, skyFBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);


		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glBindFramebuffer(GL_FRAMEBUFFER, skyFbo);
		glUseProgram(skyShader);
		labhelper::setUniformSlow(skyShader, "projection", projMatrix);
		labhelper::setUniformSlow(skyShader, "view", viewMatrix);
		labhelper::setUniformSlow(skyShader, "sunAngle", rotateSun(sunangle));
		labhelper::setUniformSlow(skyShader, "cameraPosition", cameraPosition);
		labhelper::setUniformSlow(skyShader, "cameraPosition", cameraDirection);
		labhelper::setUniformSlow(skyShader, "invViewProjection", glm::inverse(projMatrix * viewMatrix));

		labhelper::drawFullScreenQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Set uniforms for testshader. 
		glUseProgram(testShader); 
		labhelper::setUniformSlow(testShader, "viewPos", cameraPosition);
		labhelper::setUniformSlow(testShader, "lightDirection", lightDirection);
		labhelper::setUniformSlow(testShader, "lightDiffuse", glm::vec3(1.0f));
		labhelper::setUniformSlow(testShader, "lightAmbient", glm::vec3(0.1f));
		labhelper::setUniformSlow(testShader, "lightSpecular", glm::vec3(1.0f));
		labhelper::setUniformSlow(testShader, "materialShininess", (1.0f));
		labhelper::setUniformSlow(testShader, "projection", projMatrix);
		labhelper::setUniformSlow(testShader, "view", viewMatrix);
		labhelper::setUniformSlow(testShader, "model", gridMatrix);
		//labhelper::setUniformSlow(testShader, "materialColor", vec3(1.0f, 0.0f, 0.0f));
		labhelper::setUniformSlow(testShader, "model", gridMatrix);
		labhelper::setUniformSlow(testShader, "waterPlaneHeight", waterHeight); // Height if the water plane (default 0.0)
		labhelper::setUniformSlow(testShader, "waterPlaneDirection", 1.0f); // 1 = cull > height, -1 = cull < height
		
		
	
		// Set uniforms for waterShader
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
		labhelper::setUniformSlow(waterShader, "lightDiffuse", glm::vec3(1.0f));
		labhelper::setUniformSlow(waterShader, "lightAmbient", glm::vec3(0.1f));
		labhelper::setUniformSlow(waterShader, "lightSpecular", glm::vec3(1.0f));
		labhelper::setUniformSlow(waterShader, "materialShininess", (1.0f));


		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CLIP_DISTANCE0);
	
		// Wave movement
		waveScale += waveSpeed;
		if (waveScale >= 8.0) {
			waveScale -= 1.0;
		}
		labhelper::setUniformSlow(waterShader, "waveScale", waveScale);

		// ground, reflection (water)
		glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
		glBindTexture(GL_TEXTURE_2D, reflectionTextureColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindRenderbuffer(GL_RENDERBUFFER, reflectionFBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
		
		// ground, refraction
		glBindTexture(GL_TEXTURE_2D, refractionTextureColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, refractionTextureDepthbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, windowWidth, windowHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glBindRenderbuffer(GL_RENDERBUFFER, refractionFBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		glUseProgram(testShader);
		// Set textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GrassTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, StoneTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, SandTexture);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, SnowTexture);

		


		// Change camera for the reflection
		float cameraDistance = 2 * (cameraPosition.y - waterHeight);

		cameraPosition = { cameraPosition.x, cameraPosition.y - cameraDistance, cameraPosition.z };
		cameraDirection.y = -cameraDirection.y;
		labhelper::setUniformSlow(testShader, "viewPos", cameraPosition);
		projMatrix = glm::perspective(glm::radians(45.0f), float(windowWidth) / float(windowHeight), nearPlane, farPlane); // 0.1f, 2000000.0f near/far plane
		viewMatrix = lookAt(cameraPosition, cameraPosition + cameraDirection, worldUp);
		labhelper::setUniformSlow(testShader, "projection", projMatrix);
		labhelper::setUniformSlow(testShader, "view", viewMatrix);


		// Draw Sky
		glUseProgram(skyShader);
		labhelper::setUniformSlow(skyShader, "waterPlaneDirection", -1.0f);
		labhelper::drawFullScreenQuad();
		// Draw Grid
		glUseProgram(testShader);
		labhelper::setUniformSlow(testShader, "waterPlaneDirection", -1.0f);
		grid.DrawGrid();


		cameraPosition = { cameraPosition.x, cameraPosition.y + cameraDistance, cameraPosition.z };
		cameraDirection.y = -cameraDirection.y;
		labhelper::setUniformSlow(testShader, "viewPos", cameraPosition);
		projMatrix = glm::perspective(glm::radians(45.0f), float(windowWidth) / float(windowHeight), nearPlane, farPlane);
		viewMatrix = lookAt(cameraPosition, cameraPosition + cameraDirection, worldUp);
		labhelper::setUniformSlow(testShader, "projection", projMatrix);
		labhelper::setUniformSlow(testShader, "view", viewMatrix);

		// ground, refraction (water)
		glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
		// Draw sky		
		glUseProgram(skyShader);
		labhelper::setUniformSlow(skyShader, "waterPlaneDirection", 1.0f);
		labhelper::drawFullScreenQuad();
		// Draw grid
		glUseProgram(testShader);
		labhelper::setUniformSlow(testShader, "waterPlaneDirection", 1.0f);
		grid.DrawGrid();


		// Draw scene regularly
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Draw sky
		glUseProgram(skyShader);
		labhelper::setUniformSlow(skyShader, "waterPlaneDirection", 0.0f);
		labhelper::drawFullScreenQuad();
		// Draw grid
		glUseProgram(testShader);
		labhelper::setUniformSlow(testShader, "waterPlaneDirection", 0.0f);
		grid.DrawGrid();


		// Render waterPlane
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
		labhelper::setUniformSlow(waterShader, "lightDiffuse", glm::vec3(1.0f));
		labhelper::setUniformSlow(waterShader, "lightAmbient", glm::vec3(0.1f));
		labhelper::setUniformSlow(waterShader, "lightSpecular", glm::vec3(1.0f));
		labhelper::setUniformSlow(waterShader, "materialShininess", (1.0f));

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
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, skyTexture);
		
		
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		//glDisable(GL_BLEND);
		
		glBindVertexArray(0);
	



		// Set uniforms for compute shader
		computeShader.use();
		computeShader.setInt("size", gridSize);
		computeShader.setFloat("simpexScale", simpexScale);
		computeShader.setFloat("worleyScale", worleyScale);
		computeShader.setFloat("simpexAmplitude", simpexAmplitude);
		computeShader.setFloat("worleyAmplitude", worleyAmplitude);
		computeShader.setFloat("ratio", ratio);
		computeShader.setVec3("translation", gridSpeed);
		
		// SSBO for computeshader
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, grid.getVBO());
		glDispatchCompute(57600, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);



	

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
