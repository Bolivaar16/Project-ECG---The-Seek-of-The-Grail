#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include "Graphics\window.h"
#include "Camera\camera.h"
#include "Shaders\shader.h"
#include "Model Loading\mesh.h"
#include "Model Loading\texture.h"
#include "Model Loading\meshLoaderObj.h"
#include <Physics/BoundingBox.h>
#include <Player/Projectile.h>
#include <Logic/GameManager.h>
#include <Player/Enemy.h>
#include "Resources/Imgui/imgui.h"
#include "Resources/Imgui/imgui_impl_glfw.h"
#include "Resources/Imgui/imgui_impl_opengl3.h"
#include <Player/QuestItem.h>
#include <../glm/gtc/matrix_transform.hpp>
#include <../glm/gtc/type_ptr.hpp>
#include <../glm/glm.hpp>

void processKeyboardInput();

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

Window window("The Seek of the Grail", 800, 800);
Camera camera;

//Collision boxes
BoundingBox playerBox;
BoundingBox enemyBox;
GameManager gameManager;

struct Castle {
	glm::vec3 position;
	BoundingBox box;

	Castle() {}
	Castle(glm::vec3 pos, glm::vec3 size) {
		position = pos;
		box = BoundingBox(pos, size);
	}
};

Castle castle;

//Meshes
Mesh tree;
Mesh rock;
Mesh pillar;
Mesh castleMesh;

//Flags
bool bossHasSpawned = false;
bool grailLanded = false;

// Projectiles container
std::vector<Projectile> fireballs;
// Enemies container
std::vector<Enemy> enemies;
//Quest Items
QuestItem staff(glm::vec3(95.0f, -15.0f, -809.0f), glm::vec3(15.0f, 50.0f, 15.0f));
QuestItem grail(glm::vec3(0.0f, 0.0f, -20.0f), glm::vec3(3.5f, 3.5f, 3.5f));
glm::vec3 bossDeathPosition;

glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(-180.0f, 500.0f, -200.0f);

std::vector<glm::vec3> ruinsPositions = {
	glm::vec3(5.0f, 0.0f,  5.0f),
	glm::vec3(-5.0f, 0.0f,  5.0f),
	glm::vec3(-8.0f, 0.0f, -8.0f),
	glm::vec3(8.0f, 0.0f, -6.0f),
	glm::vec3(0.0f, 0.0f, -10.0f),
	glm::vec3(2.0f, 0.0f,  15.0f)
};

float getRandomRange(float min, float max) {
	return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}

void resetGame() {
	std::cout << "Restarting Game..." << std::endl;

	gameManager = GameManager();
	camera = Camera();

	// 1. Reset Enemies
	enemies.clear();

	// Spawn ONLY the zombies initially
	for (int i = 0; i < 5; i++) {
		float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 6.2831f;

		float radius = 500.0f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 500.0f);

		float offsetX = cos(angle) * radius;
		float offsetZ = sin(angle) * radius;

		glm::vec3 playerPos = camera.getCameraPosition();
		glm::vec3 pos = glm::vec3(playerPos.x + offsetX, 0.0f, playerPos.z + offsetZ);

		enemies.push_back(Enemy(pos, EnemyType::ZOMBIE));
	}

	bossHasSpawned = false;
	grailLanded = false;

	// 2. Reset Quest Items
	staff = QuestItem(glm::vec3(95.0f, -15.0f, -809.0f), glm::vec3(15.0f, 50.0f, 15.0f));
	staff.isActive = true;
	staff.isCollected = false;

	grail = QuestItem(glm::vec3(0.0f, 0.0f, -20.0f), glm::vec3(3.5f, 3.5f, 3.5f));
	grail.isActive = false;
	grail.isCollected = false;

	castle = Castle(glm::vec3(0.0f, -20.0f, 225.0f), glm::vec3(100.0f, 300.0f, 100.0f));

	fireballs.clear();

	lastFrame = glfwGetTime();
	deltaTime = 0.0f;
	gameManager.isGameOver = false;
}

std::vector<glm::vec3> mountainsPositions = {
	glm::vec3(1200.0f, -20.5f,  1200.0f),
	glm::vec3(-1200.0f, -20.5f,  1200.0f),
	glm::vec3(-1500.0f, -20.5f, -1500.0f),
	glm::vec3(1500.0f, -20.5f, -1300.0f),
	glm::vec3(700.0f, -20.5f, -1700.0f),
	glm::vec3(900.0f, -20.5f,  2200.0f)
};

glm::vec3 treePos[] = {
	glm::vec3(150.0f, -20.0f, 150.0f),
	glm::vec3(-200.0f, -20.0f, -100.0f),
	glm::vec3(250.0f, -20.0f, -250.0f),
	glm::vec3(100.0f, -20.0f,  400.0f),
	glm::vec3(560.0f, -20.0f,   100.0f),
	glm::vec3(-300.0f, -20.0f,  360.0f),
	glm::vec3(-560.0f, -20.0f,  500.0f),
	glm::vec3(-160.0f, -20.0f,   160.0f),
	glm::vec3(-400.0f, -20.0f, -600.0f),
	glm::vec3(-600.0f, -20.0f, -594.0f),
	glm::vec3(453.0f, -20.0f, -533.0f),
	glm::vec3(500.0f, -20.0f, -600.0f),
	glm::vec3(0.0f, -20.0f, -320.0f),
	glm::vec3(320.0f, -20.0f,  320.0f),
	glm::vec3(-300.0f, -20.0f,   20.0f)
};

glm::vec3 rocksPositions[] = {
	// Esquinas lejanas
				glm::vec3(500.0f, -20.5f,  500.0f),
				glm::vec3(-550.0f, -20.5f,  450.0f),
				glm::vec3(-600.0f, -20.5f, -600.0f),
				glm::vec3(650.0f, -20.5f, -550.0f),

				// Puntos cardinales lejanos
				glm::vec3(0.0f, -20.5f,  800.0f),
				glm::vec3(800.0f, -20.5f,    0.0f),
				glm::vec3(-750.0f, -20.5f,  -50.0f),
				glm::vec3(100.0f, -20.5f, -700.0f),

				// --- NUEVAS (Para rellenar huecos) ---
				// Zona Norte
				glm::vec3(400.0f, -20.5f,  750.0f),
				glm::vec3(-350.0f, -20.5f,  700.0f),

				// Zona Sur
				glm::vec3(450.0f, -20.5f, -750.0f),
				glm::vec3(-450.0f, -20.5f, -650.0f),

				// Zona Este
				glm::vec3(750.0f, -20.5f,  300.0f),
				glm::vec3(850.0f, -20.5f, -300.0f),

				// Zona Oeste
				glm::vec3(-800.0f, -20.5f,  300.0f),
				glm::vec3(-700.0f, -20.5f, -300.0f),

				// Extras aleatorias
				glm::vec3(600.0f, -20.5f,  100.0f),
				glm::vec3(-600.0f, -20.5f,  100.0f),
				glm::vec3(100.0f, -20.5f,  600.0f),
				glm::vec3(-100.0f, -20.5f, -600.0f)
};

int main()
{
	srand(static_cast<unsigned int>(time(0)));
	glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

	//building and compiling shader program
	Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
	Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");

	//Textures
	GLuint texWood = loadBMP("Resources/Textures/wood.bmp");
	GLuint tex2 = loadBMP("Resources/Textures/rock.bmp");
	GLuint tex3 = loadBMP("Resources/Textures/orange.bmp");
	GLuint tex4 = loadBMP("Resources/Textures/rock_real.bmp");
	GLuint tex5 = loadBMP("Resources/Textures/black.bmp");
	GLuint tex6 = loadBMP("Resources/Textures/castle.bmp");
	GLuint texZombie = loadBMP("Resources/Textures/zombi.bmp");
	GLuint texGrail = loadBMP("Resources/Textures/Grail_Grail_BaseColor.bmp");

	glEnable(GL_DEPTH_TEST);

	// Initialize ImGui Context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// Setup ImGui style
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 400");

	

	std::vector<Texture> textures2;
	textures2.push_back(Texture());
	textures2[0].id = tex2;
	textures2[0].type = "texture_diffuse";

	std::vector<Texture> textures;
	textures.push_back(Texture());
	textures[0].id = texWood;
	textures[0].type = "texture_diffuse";

	std::vector<Texture> textures3;
	textures3.push_back(Texture());
	textures3[0].id = tex3;
	textures3[0].type = "texture_diffuse";

	std::vector<Texture> textures4;
	textures4.push_back(Texture());
	textures4[0].id = tex4;
	textures4[0].type = "texture_diffuse";

	std::vector<Texture> textures5;
	textures5.push_back(Texture());
	textures5[0].id = tex5;
	textures5[0].type = "texture_diffuse";

	std::vector<Texture> textures6;
	textures6.push_back(Texture());
	textures6[0].id = tex6;
	textures6[0].type = "texture_diffuse";

	std::vector<Texture> zombieTextures;
	zombieTextures.push_back(Texture());
	zombieTextures[0].id = texZombie;
	zombieTextures[0].type = "texture_diffuse";

	std::vector<Texture> grailTextures;
	grailTextures.push_back(Texture());
	grailTextures[0].id = texGrail;
	grailTextures[0].type = "texture_diffuse";


	//Mesh mesh(vert, ind, textures3);

	MeshLoaderObj loader;
	Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
	Mesh plane = loader.loadObj("Resources/Models/plane.obj", textures2);
	Mesh realRock = loader.loadObj("Resources/Models/rock_real.obj", textures4);
	Mesh deadTree = loader.loadObj("Resources/Models/dead_tree.obj", textures5);
	Mesh castleMesh = loader.loadObj("Resources/Models/castle.obj", textures6);
	Mesh zombieMesh = loader.loadObj("Resources/Models/zombi.obj", zombieTextures);
	Mesh grailMesh = loader.loadObj("Resources/Models/Grail.obj", grailTextures);
	Mesh wandMesh = loader.loadObj("Resources/Models/Wand.obj", textures);

	// Initialization
	// Player
	playerBox = BoundingBox(camera.getCameraPosition(), glm::vec3(100.0f, 200.0f, 100.0f));

	//Grail
	grail.isActive = false;

	// Zombi

	for (int i = 0; i < 5; i++) {
		float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 6.2831f;

		float radius = 500.0f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 500.0f);

		float offsetX = cos(angle) * radius;
		float offsetZ = sin(angle) * radius;

		glm::vec3 playerPos = camera.getCameraPosition();
		glm::vec3 pos = glm::vec3(playerPos.x + offsetX, 0.0f, playerPos.z + offsetZ);

		enemies.push_back(Enemy(pos, EnemyType::ZOMBIE));
	}

	//enemies.push_back(Enemy(glm::vec3(0.0f, 0.0f, -30.0f), EnemyType::BOSS));

	//Castle
	castle = Castle(glm::vec3(0.0f, -20.0f, 225.0f), glm::vec3(100.0f, 300.0f, 100.0f));


	//check if we close the window or press the escape button
	while (!window.isPressed(GLFW_KEY_ESCAPE) &&
		glfwWindowShouldClose(window.getWindow()) == 0)
	{
		window.clear();
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (!gameManager.isGameOver && !gameManager.gameFinished) {
			processKeyboardInput();

			//test mouse input
			if (window.isMousePressed(GLFW_MOUSE_BUTTON_LEFT))
			{
				std::cout << "Pressing mouse button" << std::endl;
			}

			//Phisics and logic update

			playerBox.update(camera.getCameraPosition() - glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 2.0f, 1.0f));

			for (auto& enemy : enemies) {
				enemy.update(deltaTime, camera.getCameraPosition());
			}

			for (auto& ball : fireballs) {
				ball.update(deltaTime);
			}

			gameManager.update(camera.getCameraPosition(), enemies);
			if (gameManager.currentTaskIndex == 2 && !bossHasSpawned) {

				std::cout << "THE BOSS HAS AWAKENED!" << std::endl;

				// Spawn the Boss at a specific location
				enemies.push_back(Enemy(glm::vec3(0.0f, 0.0f, -30.0f), EnemyType::BOSS));

				bossHasSpawned = true; // Lock so we don't spawn him infinitely
			}
			// --- QUEST LOGIC UPDATE ---

			if (gameManager.grailSpawned && !grail.isCollected) {
				if (!grail.isActive) {
					grail.isActive = true;
					grail.position.y = 200.0f;
					grailLanded = false;
				}
			}

			if (grail.isActive && !grailLanded) {
				float descentSpeed = 50.0f;
				grail.position.y -= descentSpeed * deltaTime;

				if (grail.position.y <= 3.0f) {
					grail.position.y = 3.0f;
					grailLanded = true;
				}
			}

			// --- QUEST ITEM COLLISIONS ---

			// Staff Collision
			if (staff.isActive && playerBox.checkCollision(staff.box)) {
				std::cout << "Staff collected!" << std::endl;

				staff.isCollected = true;
				staff.isActive = false; // Hide it

				if (gameManager.currentTaskIndex == 0) {
					gameManager.tasks[0].isCompleted = true;
				}
			}

			// Grail Collision
			if (grail.isActive && grailLanded && playerBox.checkCollision(grail.box)) {
				std::cout << "Holy Grail collected!" << std::endl;

				grail.isCollected = true;
				grail.isActive = false; // Hide it (put in inventory)

				if (gameManager.currentTaskIndex == 3) {
					gameManager.tasks[3].isCompleted = true;

					gameManager.grailSpawned = false;
				}
			}

			//Castle Collision
			if (grail.isCollected && playerBox.checkCollision(castle.box)) {

				// Check if we are on the final mission (Index 4)
				if (gameManager.currentTaskIndex == 4) {
					std::cout << "Victory! Returned to Camelot." << std::endl;

					gameManager.tasks[4].isCompleted = true;
					gameManager.gameFinished = true;
				}
			}
		}
		else if (gameManager.isGameOver || gameManager.gameFinished) {

			static bool enterPressedLastFrame = false;
			bool enterPressedNow = window.isPressed(GLFW_KEY_ENTER);

			if (enterPressedNow && !enterPressedLastFrame) {
				resetGame();
				lastFrame = glfwGetTime();
				deltaTime = 0.0f;
			}

			enterPressedLastFrame = enterPressedNow;
		}

		// Check collisions

		for (auto& enemy : enemies) {
			if (enemy.isDead) continue;

			if (playerBox.checkCollision(enemy.box)) {
				std::cout << "GAME OVER: Touched by enemy!" << std::endl;
				gameManager.isGameOver = true; // Trigger Defeat
			}
		}


		for (auto& ball : fireballs) {
			if (!ball.isActive && staff.isCollected) continue;

			for (auto& enemy : enemies) {
				if (enemy.isDead) continue;

				if (ball.box.checkCollision(enemy.box)) {
					std::cout << "Enemy Hit!" << std::endl;

					// Damage the enemy
					enemy.takeDamage(50.0f);
					if (enemy.type == EnemyType::BOSS && enemy.isDead) {
						bossDeathPosition = enemy.position;
					}

					// Destroy the fireball
					ball.isActive = false;
					break;
				}
			}
		}

		//// Code for the light ////

		sunShader.use();

		glm::mat4 ProjectionMatrix = glm::perspective(90.0f, window.getWidth() * 1.0f / window.getHeight(), 0.1f, 10000.0f);
		glm::mat4 ViewMatrix = glm::lookAt(camera.getCameraPosition(), camera.getCameraPosition() + camera.getCameraViewDirection(), camera.getCameraUp());

		GLuint MatrixID = glGetUniformLocation(sunShader.getId(), "MVP");

		GLuint ObjectColorID = glGetUniformLocation(sunShader.getId(), "objectColor");

		//Test for one Obj loading = light source

		glm::mat4 ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, lightPos);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		//White moon
		glUniform3f(ObjectColorID, 1.0f, 1.0f, 1.0f);

		sun.draw(sunShader);

		// Draw fireballs
		for (auto& ball : fireballs)
		{
			if (ball.isActive && staff.isCollected)
			{
				glm::mat4 ModelMatrix = glm::mat4(1.0);
				ModelMatrix = glm::translate(ModelMatrix, ball.position);
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.2f));

				glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

				glUniform3f(ObjectColorID, 1.0f, 0.5f, 0.0f);

				sun.draw(sunShader);
			}
		}


		// Draw Quest Items

		

		// Draw Grail
		if (grail.isActive) {
			glm::vec3 drawPos = grail.position;

			if (grailLanded) {
				float time = glfwGetTime();
				drawPos.y += sin(time * 2.0f) * 0.5f;
			}

			ModelMatrix = glm::mat4(1.0);
			ModelMatrix = glm::translate(ModelMatrix, drawPos);
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f));
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniform3f(ObjectColorID, 1.0f, 0.84f, 0.0f);
			grailMesh.draw(shader);

			float verticalSpread = 60.0f;
			float rayTime = glfwGetTime();
			for (int i = 0; i < 8; i++) {
				glm::mat4 RayMatrix = glm::mat4(1.0);
				RayMatrix = glm::translate(RayMatrix, drawPos);

				float angleY = (rayTime * 100.0f) + (i * 30.0f);
				RayMatrix = glm::rotate(RayMatrix, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));

				float angleX = (i % 2 == 0) ? verticalSpread : -verticalSpread;
				RayMatrix = glm::rotate(RayMatrix, glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f));

				RayMatrix = glm::scale(RayMatrix, glm::vec3(0.02f, 0.02f, 2.5f));

				MVP = ProjectionMatrix * ViewMatrix * RayMatrix;
				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

				glUniform3f(ObjectColorID, 1.0f, 1.0f, 0.6f);
				sun.draw(sunShader);
			}
		}


		shader.use();

		// Draw Staff
		if (staff.isActive) {
			ModelMatrix = glm::mat4(1.0);
			ModelMatrix = glm::translate(ModelMatrix, staff.position);
			ModelMatrix = glm::rotate(ModelMatrix, 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f));
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			wandMesh.draw(shader);
		}

		///// Test Obj files for box ////

		GLuint MatrixID2 = glGetUniformLocation(shader.getId(), "MVP");
		GLuint ModelMatrixID = glGetUniformLocation(shader.getId(), "model");

		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -2.0f, 0.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		///// Plane
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -20.0f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(100.0f, 1.0f, 100.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex2);

		plane.draw(shader);

		// Draw enemies
		for (auto& enemy : enemies) {
			if (enemy.isDead) continue;

			ModelMatrix = glm::mat4(1.0);

			glm::vec3 adjustedPos = enemy.position;
			adjustedPos.y -= 20.0f;

			ModelMatrix = glm::translate(ModelMatrix, adjustedPos);

			ModelMatrix = glm::rotate(ModelMatrix, -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));

			glm::vec3 direction = glm::normalize(camera.getCameraPosition() - enemy.position);
			float angle = atan2(direction.x, direction.z);
			ModelMatrix = glm::rotate(ModelMatrix, angle * 50, glm::vec3(0.0f, 0.0f, 1.0f));

			if (enemy.type == EnemyType::BOSS) {
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(30.0f));
			}
			else {
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(10.0f));
			}

			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			zombieMesh.draw(shader);
		}

		for (int i = 0; i < mountainsPositions.size(); i++)
		{
			ModelMatrix = glm::mat4(1.0);
			ModelMatrix = glm::translate(ModelMatrix, mountainsPositions[i]);
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(300.5f, 300.5f, 300.5f));

			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

			glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

			realRock.draw(shader);
		}


		for (int i = 0; i < 15; i++)
		{
			ModelMatrix = glm::mat4(1.0);
			ModelMatrix = glm::translate(ModelMatrix, treePos[i]);

			// Escala x2 (ajusta si se ve muy grande o chico)
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(20.0f, 20.0f, 20.0f));

			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

			glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

			// Dibujamos el árbol tal cual
			deadTree.draw(shader);
		}

		//CASTLE
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, castle.position);

		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(40.0f, 60.0f, 20.0f));

		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		castleMesh.draw(shader);

		for (int i = 0; i < 8; i++)
		{
			ModelMatrix = glm::mat4(1.0);
			ModelMatrix = glm::translate(ModelMatrix, rocksPositions[i]);

			float scaleSize = (i % 2 == 0) ? 90.0f : 45.0f;

			if (i % 3 == 0) scaleSize = 25.0f;

			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(scaleSize, scaleSize, scaleSize));

			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

			glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

			realRock.draw(shader);
		}
		// ImGui Rendering Logic
		// ---------------------------------------------------------

		// Start a new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Define the HUD Window properties
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav;

		ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
		ImGui::SetNextWindowBgAlpha(0.35f);

		//Begin the ImGui Window
		if (ImGui::Begin("TaskOverlay", NULL, window_flags))
		{
			if (gameManager.isGameOver) {
				// DEFEAT SCREEN UI
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "YOU DIED");
				ImGui::Text("The darkness has consumed you.");
				ImGui::Separator();
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Press ENTER to restart the game");
			}
			else if (gameManager.gameFinished) {
				// VICTORY SCREEN UI
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "VICTORY!");
				ImGui::Text("Camelot is saved.");
			}
			else {
				// NORMAL HUD
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "CURRENT OBJECTIVE:");
				std::string currentTask = gameManager.getCurrentTaskInfo();
				ImGui::Text("%s", currentTask.c_str());
			}
		}
		ImGui::End();

		//Render ImGui data onto the screen
		glDisable(GL_DEPTH_TEST);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glEnable(GL_DEPTH_TEST);
		window.update();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void processKeyboardInput()
{
	float cameraSpeed = 100 * deltaTime;

	//translation
	if (window.isPressed(GLFW_KEY_W))
		camera.keyboardMoveFront(cameraSpeed);
	if (window.isPressed(GLFW_KEY_S))
		camera.keyboardMoveBack(cameraSpeed);
	if (window.isPressed(GLFW_KEY_A))
		camera.keyboardMoveLeft(cameraSpeed);
	if (window.isPressed(GLFW_KEY_D))
		camera.keyboardMoveRight(cameraSpeed);
	if (window.isPressed(GLFW_KEY_R)) {
		glm::vec3 playerPos = camera.getCameraPosition();
		std::cout << "x:" << playerPos.x << "y:" << playerPos.y << "z:" << playerPos.z << std::endl;
	}
	// Shooting mechanic
	static float lastShotTime = 0.0f;
	float currentTime = glfwGetTime();
	if (window.isPressed(GLFW_KEY_F) && (currentTime - lastShotTime > 0.5f) && staff.isCollected) // 0.5s cooldown
	{
		// Spawn fireball at camera position, moving in camera direction
		fireballs.push_back(Projectile(camera.getCameraPosition() - glm::vec3(10.0f, 0.5f, 10.0f), camera.getCameraViewDirection()));
		lastShotTime = currentTime;
		std::cout << "Fireball cast!" << std::endl;
	}


	float rotationSpeed = 60 * deltaTime;

	//rotation
	if (window.isPressed(GLFW_KEY_LEFT))
		camera.rotateOy(rotationSpeed);
	if (window.isPressed(GLFW_KEY_RIGHT))
		camera.rotateOy(-rotationSpeed);
	if (window.isPressed(GLFW_KEY_UP))
		camera.rotateOx(rotationSpeed);
	if (window.isPressed(GLFW_KEY_DOWN))
		camera.rotateOx(-rotationSpeed);
}