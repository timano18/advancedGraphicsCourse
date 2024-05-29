
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
#include "grass.h"

#define PI 3.14159265



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
GLuint grass_shader;




///////////////////////////////////////////////////////////////////////////////
// Light source
///////////////////////////////////////////////////////////////////////////////
glm::vec3 lightPosition = glm::vec3(0.0f, 813.0f, 752.0f);
float sunangle = 0.0f;
glm::vec3 lightDirection;

///////////////////////////////////////////////////////////////////////////////
// Camera parameters.
///////////////////////////////////////////////////////////////////////////////
glm::vec3 cameraPosition(-0.1f, 130.0f, -0.1f);
glm::vec3 cameraDirection = glm::normalize(glm::vec3(199.0f, 80.0f, 327.0f) - cameraPosition);

float farPlane = 2000000.0f;
float nearPlane = 0.1f;

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



bool followGrid = true;

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
	shader = labhelper::loadShaderProgram("../project/grass_shader.vert", "../project/grass_shader.frag", is_reload);
	if (shader != 0)
	{
		grass_shader = shader;
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
	if(state[SDL_SCANCODE_Q] || state[SDL_SCANCODE_LCTRL])
	{
		cameraPosition -= cameraSpeed * deltaTime * worldUp;
	}
	if(state[SDL_SCANCODE_E] || state[SDL_SCANCODE_SPACE])
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

unsigned int GrassPicture;

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

	// Load "Grass_Picture"

	glGenTextures(1, &GrassPicture);
	glBindTexture(GL_TEXTURE_2D, GrassPicture);

	int GrassPWidth, GrassPHeight, GrassPNrChannels;
	unsigned char* GrassPData = stbi_load("../textures/grass/stolen_grass2.png", &GrassPWidth, &GrassPHeight, &GrassPNrChannels, 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GrassPWidth, GrassPHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, GrassPData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_2D);

}


int main(int argc, char* argv[])
{
	

	g_window = labhelper::init_window_SDL("OpenGL Project");

	
	initialize();

	bool stopRendering = false;

	auto startTime = std::chrono::system_clock::now();																				// Start clock, create inition chunks
	auto start = std::chrono::high_resolution_clock::now();													

	// Generate initial grid parameters
	int gridWidth = 580;
	int gridHeight = 580;
	int xStartPos;
	int yStartPos;
	float cellSize = 10.0;
	float perlinScale = 750.0;
	float voronoiScale = 100.0;

	// Generate initial chunk parameters
	int xChunkStart = 0;
	int yChunkStart = 0;
	int xChunkEnd = 1;
	int yChunkEnd = 1;

	int LoD = 8; // Tillfällig

	//GridChunk initialChunk1;
	//GridChunk initialChunk2;

	// query limitations
	computeShaderSetupQuery();
	


	// Build and compile shaders
	ComputeShader computeShader("../project/simpleComputeShader.glsl");





	Grid grid;
	grid.generateGrid();

	glBindBuffer(GL_ARRAY_BUFFER, grid.getVBO());
	Vertex* mappedVertices = (Vertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	for (int i = 0; i < 10; ++i) {
		printf("Vertex %d: Pos(%f, %f, %f), Norm(%f, %f, %f)\n",
			i,
			mappedVertices[i].position.x, mappedVertices[i].position.y, mappedVertices[i].position.z,
			mappedVertices[i].normal.x, mappedVertices[i].normal.y, mappedVertices[i].normal.z);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
																																	// *** KOMMENTARER: Lägg till "levels of detail". Går ej att stoppa in negativa koordinater just nu. Kanske borde byta från (x1,y1,x2,y2) till (x1,x2,y1,y2)? Fixa "GridChunk::generateChunkGrids". Gör klart "GridChunk::gridChunkCenter()"
	// createNewStandardChunk(xChunkStart, yChunkStart, xChunkEnd, yChunkEnd);																	// Standard (värden i grid.cpp)
	//initialChunk1.createNewStandardChunk(0, 0, 1, 1);
	// createNewChunk(xChunkStart, yChunkStart, xChunkEnd, yChunkEnd, gridWidth, gridHeight, cellSize, perlinScale, voronoiScale);				// Välj variabler
	//initialChunk2.createNewChunk(0, 1, 3, 2, gridWidth / LoD, gridHeight / LoD, cellSize * LoD, perlinScale, voronoiScale);						// Artificiellt lägre LoD. Måste ändra på filter-variablerna också för att det ska bli korrekt?

	gridMatrix = glm::mat4(1.0f);

	//gridMatrix = glm::rotate(gridMatrix, glm::radians(90.0f), glm::vec3(1, 0, 0));
	//gridMatrix = glm::translate(gridMatrix, glm::vec3(-gridWidth / 2.0f, -gridHeight / 2.0f, -1500.0f));
	
	//cameraPosition = glm::vec3(gridMatrix[3]);

	gridSpeed = glm::vec3(0.0f, 0.0f, 0.0f); // Grid position
	glm::mat4 initialRotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(90.0f, 0.0f, 0.0f)); // Example rotation
	gridMatrix = initialRotation; // Applying initial rotation


	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::duration<float>>(stop - start);
	std::cout << "Initial grid(s); generation time: " << duration.count() << std::endl;												// End clock, create inition chunks


	
	std::cout << "After compute shader" << std::endl;
	glBindBuffer(GL_ARRAY_BUFFER, grid.getVBO());
	mappedVertices = (Vertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	for (int i = 0; i < 100; ++i) {
		printf("Vertex %d: Pos(%f, %f, %f), Norm(%f, %f, %f)\n",
			i,
			mappedVertices[i].position.x, mappedVertices[i].position.y, mappedVertices[i].position.z,
			mappedVertices[i].normal.x, mappedVertices[i].normal.y, mappedVertices[i].normal.z);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);





	// Buffer for vertexFollow
	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3), NULL, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	
	// Textures
	loadTextures();

	// START GRASS


	//Grass grass1(1000, 1000, 0, 0, 0);
	//grass1.generateGrassSquare();

	Grass grass2(100, 100, 0, 0, 10000, 0);
	grass2.generateGrassStar();

	//Grass grass3(1000, 1000, 10000, 10000, 0);
	//grass3.generateGrassTriangle();

	float time = 0.0;

	// STOP GRASS



	
	// Start render-loop
	while (!stopRendering)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glm::vec3* ptr = (glm::vec3*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec3), GL_MAP_READ_BIT);

		if (ptr) {
			vertexFollow = *ptr;
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


		if (followGrid) {
			cameraPosition = glm::vec3(vertexFollow.x + 1200.0f, cameraPosition.y, vertexFollow.y + 1200.0f);
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
		glUseProgram(testShader); 
		labhelper::setUniformSlow(testShader, "viewPos", cameraPosition);
		labhelper::setUniformSlow(testShader, "lightDirection", lightDirection);
		labhelper::setUniformSlow(testShader, "lightDiffuse", glm::vec3(1.0f));
		labhelper::setUniformSlow(testShader, "lightAmbient", glm::vec3(0.1f));
		labhelper::setUniformSlow(testShader, "lightSpecular", glm::vec3(1.0f));
		labhelper::setUniformSlow(testShader, "materialShininess", (1.0f));
		//labhelper::setUniformSlow(testShader, "materialColor", vec3(1.0f, 0.0f, 0.0f));
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GrassTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, StoneTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, SandTexture);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, SnowTexture);
		

		glm::mat4 projMatrix = glm::perspective(glm::radians(45.0f), float(windowWidth) / float(windowHeight), nearPlane, farPlane);
		glm::mat4 viewMatrix = lookAt(cameraPosition, cameraPosition + cameraDirection, worldUp);
		labhelper::setUniformSlow(testShader, "projection", projMatrix);
		labhelper::setUniformSlow(testShader, "view", viewMatrix);
		labhelper::setUniformSlow(testShader, "model", gridMatrix);
		//grid.DrawGrid();
	

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
		//glDispatchCompute(57600, 1, 1);


		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		// GRASS START

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUseProgram(grass_shader);

		// count up time (for wind)
		time = time + 0.1f * (PI / 180.0f); // Degrees
		if (time > (180 * (PI / 180.0f))) {
			time = -180.0f * (PI / 180.0f);
		}
		//std::cout << time / (PI / 180.0f) << " degrees" << '\n';

		labhelper::setUniformSlow(grass_shader, "viewPos", cameraPosition);
		labhelper::setUniformSlow(grass_shader, "lightDirection", lightDirection);
		labhelper::setUniformSlow(grass_shader, "lightDiffuse", glm::vec3(1.0f));
		labhelper::setUniformSlow(grass_shader, "lightAmbient", glm::vec3(0.1f));
		labhelper::setUniformSlow(grass_shader, "lightSpecular", glm::vec3(1.0f));
		labhelper::setUniformSlow(grass_shader, "materialShininess", (1.0f));
		labhelper::setUniformSlow(grass_shader, "projection", projMatrix);
		labhelper::setUniformSlow(grass_shader, "view", viewMatrix);
		labhelper::setUniformSlow(grass_shader, "model", gridMatrix);
		labhelper::setUniformSlow(grass_shader, "time", time);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GrassPicture);

		//grass1.DrawGrass();
		grass2.DrawGrass();
		//grass3.DrawGrass();
		// GRASS STOP

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
