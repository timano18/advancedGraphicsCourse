
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
using namespace glm;

#include <Model.h>
#include "hdr.h"
#include "fbo.h"

#include "Grid.h"
#include "noise.h"
#include "myHelper/shader_c.h"
#include "myHelper/shader_s.h"


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




///////////////////////////////////////////////////////////////////////////////
// Light source
///////////////////////////////////////////////////////////////////////////////
vec3 lightPosition = vec3(0.0f, 813.0f, 752.0f);
float sunangle = 0.0f;
vec3 lightDirection;

///////////////////////////////////////////////////////////////////////////////
// Camera parameters.
///////////////////////////////////////////////////////////////////////////////
vec3 cameraPosition(-2659.0f, 4870.0f, -3135.0f);
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
int gridSize = 3000;
float noiseScale = 0.01;




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
	ImGui::SliderFloat("NoiseScale", &noiseScale, 0.0f, 1500.0f);
	ImGui::SliderInt("Grid Size", &gridSize, 10, 10000);

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

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
};

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
	float perlinScale = 1500.0;
	float voronoiScale = 200.0;



	float LoD = 1.0; // Tillfällig

	//GridChunk initialChunk1;

	GridChunk initialChunk2;
	//GridChunk initialChunk3;
																																				// *** KOMMENTARER: Lägg till "levels of detail". Går ej att stoppa in negativa koordinater just nu. Kanske borde byta från (x1,y1,x2,y2) till (x1,x2,y1,y2)? Fixa "GridChunk::generateChunkGrids". Gör klart "GridChunk::gridChunkCenter()"
	// createNewStandardChunk(xChunkStart, yChunkStart, xChunkEnd, yChunkEnd);																	// Standard (värden i grid.cpp)
	//initialChunk1.createNewStandardChunk(0, 0, 1, 1, -500);

	//GridChunk initialChunk2;

	// query limitations
	computeShaderSetupQuery();
	


	// Build and compile shaders
	ComputeShader computeShader("../project/simpleComputeShader.glsl");


	//Grid grid;
	//grid.generateGrid();

	glBindBuffer(GL_ARRAY_BUFFER, grid.getVBO());
	Vertex* mappedVertices = (Vertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	for (int i = 0; i < 10; ++i) {
		printf("Vertex %d: Pos(%f, %f, %f), Norm(%f, %f, %f)\n",
			i,
			mappedVertices[i].position.x, mappedVertices[i].position.y, mappedVertices[i].position.z,
			mappedVertices[i].normal.x, mappedVertices[i].normal.y, mappedVertices[i].normal.z);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	

	// createNewChunk(xChunkStart, yChunkStart, xChunkEnd, yChunkEnd, gridWidth, gridHeight, cellSize, perlinScale, voronoiScale);				// Välj variabler
	initialChunk2.createNewChunk(0, 0, 4, 4, gridWidth / LoD, gridHeight / LoD, cellSize * LoD, 900, perlinScale, voronoiScale);				// Artificiellt lägre LoD. Måste ändra på filter-variablerna också för att det ska bli korrekt?
	//initialChunk3.createNewChunk(1, 2, 4, 3, gridWidth / LoD, gridHeight / LoD, cellSize * LoD, 0, perlinScale, voronoiScale);

	//for (int i = -240; i < 240; i++) {
	//	std::cout << "Perlin: " << perlinNoiseGen((float)i * 0.33, (float)i * 0.33) << '\n';
	//}


	// Ocean
	//initialChunk3.createNewChunk(-2, -2, 2, 2, gridWidth / 24, gridHeight / 24, cellSize * 24, 49.99, 0, 0);

	//initialChunk1.createNewStandardChunk(0, 0, 1, 1, 500);
	//initialChunk2.createNewStandardChunk(0, 1, 1, 2, 500);
	//initialChunk3.createNewStandardChunk(0, 2, 1, 3, 500);

	gridMatrix = mat4(1.0f);

	gridMatrix = glm::rotate(gridMatrix, glm::radians(90.0f), glm::vec3(1, 0, 0));
	gridMatrix = glm::translate(gridMatrix, glm::vec3(-gridWidth / 2.0f, -gridHeight / 2.0f, -1500.0f));
	
	//cameraPosition = glm::vec3(gridMatrix[3]);

	


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


	//glm::vec2 test = {1994,5558};
	//perturbedNoice(test);
	
	// Start render-loop
	while(!stopRendering)
	{

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
		labhelper::setUniformSlow(testShader, "lightDiffuse", vec3(1.0f));
		labhelper::setUniformSlow(testShader, "lightAmbient", vec3(0.1f));
		labhelper::setUniformSlow(testShader, "lightSpecular", vec3(1.0f));
		labhelper::setUniformSlow(testShader, "materialShininess", (1.0f));

		// Different colours for different height levels
		labhelper::setUniformSlow(testShader, "materialColor1", vec3(0.0f, 1.0f, 0.0f)); // Gräs
		labhelper::setUniformSlow(testShader, "materialColor2", vec3(0.0f, 0.0f, 1.0f)); // Vatten
		labhelper::setUniformSlow(testShader, "materialColor3", vec3(0.5f, 0.5f, 0.5f)); // Lutning
		labhelper::setUniformSlow(testShader, "materialColor4", vec3(0.7f, 0.7f, 0.5f)); // Sand
		labhelper::setUniformSlow(testShader, "materialColor5", vec3(1.0f, 1.0f, 1.0f)); // Snö
		

		mat4 projMatrix = perspective(radians(45.0f), float(windowWidth) / float(windowHeight), 0.1f, 2000000.0f);
		mat4 viewMatrix = lookAt(cameraPosition, cameraPosition + cameraDirection, worldUp);
		labhelper::setUniformSlow(testShader, "projection", projMatrix);
		labhelper::setUniformSlow(testShader, "view", viewMatrix);
		labhelper::setUniformSlow(testShader, "model", gridMatrix);
    
		// Draw initial grids to the screen
		initialChunk2.DrawGridChunk();
		//grid.DrawGrid();
	
		computeShader.use();
		computeShader.setInt("size", gridSize);
		computeShader.setFloat("noiseScale", noiseScale);
		// SSBO for computeshader
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, grid.getVBO());
		glDispatchCompute(576, 1, 1);


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
