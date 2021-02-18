#include "pch.h"
#include "GameManager.h"

#include "DebugManager.h"
#include "EntityManager.h"
#include "InputManager.h"
#include "Camera.h"
#include "KDTree.h"
#include "Fixed_Data.h"
#include "DrawKDTree.h"

#define MshMngr MeshManager::GetInstance()

#pragma region Singleton

GameManager* GameManager::instance = nullptr;

GameManager* GameManager::GetInstance()
{
    if (instance == nullptr) {
        instance = new GameManager();
    }

    return instance;
}

#pragma endregion

#pragma region Accessors

std::vector<std::shared_ptr<Light>> GameManager::GetLights()
{
    return lights;
}

std::shared_ptr<GameObject> GameManager::GetObjectByName(std::string name)
{
    for (int i = 0; i < gameObjects.size(); i++) {
        if (gameObjects[i]->GetName().compare(name) == 0) {
            return gameObjects[i];
        }
    }

    std::cout << "Could not find object with name: " << name << std::endl;
    return nullptr;
}

#pragma endregion

#pragma region Game Loop

void GameManager::Init()
{
    //Setup Lights
    lights.push_back(std::make_shared<Light>(glm::vec3(1.5f, 1.1f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 5.0f));
    lights.push_back(std::make_shared<Light>(glm::vec3(0.0f, 2.0f, -1.5f), glm::vec3(1.0f, 0.988f, 0.769f), 3.0f, 4.0f));

	float KDrange = 2.0f;

	//seed the random
	srand(time(NULL));
	//create cubes to be used by the KD tree.
	//Number of objects is set in Fixed_Data.h
	for (int i = 0; i < number_of_objects; i++) {
		
		//create random x and y coordinates for the cubes
		//range is between -2.0f and 2.0f for now
		float randX = (float) rand() / (float) RAND_MAX;
		float randRange = KDrange - -KDrange;
		float finalRandX = (randX * randRange) + -KDrange;
		float randY = (float)rand() / (float)RAND_MAX;
		float finalRandY = (randY * randRange) + -KDrange;
		
		//add a cube to the gameobject vector
		gameObjects.push_back(std::make_shared<GameObject>(EntityManager::GetInstance()->GetMeshes()[MeshTypes::Cube]));

		//set data, place position at random coords
		gameObjects[i]->SetTransform(std::make_shared<Transform>(glm::vec3()));
		gameObjects[i]->GetTransform()->SetPosition(glm::vec3(finalRandX, finalRandY, 0));
		gameObjects[i]->GetTransform()->SetOrientation(glm::vec3(0.0f, 0.0f, 0.0f));
		gameObjects[i]->GetTransform()->SetScale(glm::vec3(0.15f, 0.15f, 0.1f));
		gameObjects[i]->SetPhysicsObject(std::make_shared<PhysicsObject>(gameObjects[i]->GetTransform(), PhysicsLayers::Static, 1.0f, false, true));
		gameObjects[i]->SetName("Cube");

		//bind the points to the cube positions for the KD tree to use
		point_body_x[i] = gameObjects[i]->GetTransform()->GetPosition().x;
		point_body_y[i] = gameObjects[i]->GetTransform()->GetPosition().y;

		
		

		//bind the body points to the tree points
		//makes a check for negative and positive coords
		//then adds them to the tree point array
		//if for some reason the point is beyound the range,
		//set position as zero
		if (point_body_x[i] >= -KDrange && point_body_x[i] <= 0.00000001f) {
			point_tree_x[i] = roundf(point_body_x[i] * 1000) / 1000;
		}
		else if (point_body_x[i] >= 0.000001f && point_body_x[i] <= KDrange) {
			point_tree_x[i] = roundf(point_body_x[i] * 1000) / 1000;
		}
		else {
			point_tree_x[i] = 0;
		}

		if (point_body_y[i] >= -KDrange && point_body_y[i] <= 0.00000001f) {
			point_tree_y[i] = roundf(point_body_y[i] * 1000) / 1000;
		}
		else if (point_body_y[i] >= 0.000001f && point_body_y[i] <= KDrange) {
			point_tree_y[i] = roundf(point_body_y[i] * 1000) / 1000;
		}
		else {
			point_tree_y[i] = 0;
		}

		std::cout << " (" << point_tree_x[i] << ", " << point_tree_y[i] << ") " << "\n";

	}

	
	//loops throigh points in tree array,
	//adds to domain and range 
	//sets temp point using acquired coordinates
	//adds the temp point to the points tree vector
	//the finally adds the x and y values to the Points Coords vector
	//in the form of a _Point_xy struct
	for (int a = 0; a < number_of_objects; a++)
	{
		float _x = point_tree_x[a];
		float _y = point_tree_y[a];

		domain.push_back(_x);
		range.push_back(_y);

		point_temp.xpos = _x;
		point_temp.ypos = _y;
		//point_temp.id = a; for testing

		Points_Tree.push_back(point_temp);

		Point_Coord.push_back(_Point_xy(_x, _y));

	}

	//create the KD tree
	KD_tree* tree;
	tree = new KD_tree(Points_Tree);
	tree->printInfo();
	tree->printTree();

	//draw it using debug lines
	drawKDTree(*(tree->get_Root()));

	for (int i = 0; i < number_of_objects; i++)
	{
		std::cout << " (" << Point_Coord[i].x1 << ", " << Point_Coord[i].y1 << ") " << "\n";
	}
	
	//gameObjects.push_back(std::make_shared<GameObject>(EntityManager::GetInstance()->GetMeshes()[MeshTypes::Line]));
	gameObjects.push_back(std::make_shared<GameObject>(EntityManager::GetInstance()->GetMeshes()[MeshTypes::Skybox]));
	int lastIndex = gameObjects.size() - 1;
	int secondLastIndex = gameObjects.size() - 2;
	// setup skybox to enhnace the scene

	//DebugManager::GetInstance()->DrawLine(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), -1.0f);
	//gameObjects[secondLastIndex]->SetTransform(std::make_shared<Transform>(glm::vec3()));
	//gameObjects[secondLastIndex]->SetTransform()
	//gameObjects[secondLastIndex]->GetTransform()->SetOrientation(glm::vec3(0.0f, 0.0f, 0.0f));
	//gameObjects[secondLastIndex]->GetTransform()->SetScale(glm::vec3(1.0f, 5.0f, 0.1f));
	//gameObjects[secondLastIndex]->SetPhysicsObject(std::make_shared<PhysicsObject>(gameObjects[secondLastIndex]->GetTransform(), PhysicsLayers::Static, 1.0f, false, true));
	//gameObjects[secondLastIndex]->SetName("Cube");

	gameObjects[lastIndex]->SetTransform(std::make_shared<Transform>(glm::vec3(0)));
	gameObjects[lastIndex]->SetPhysicsObject(std::make_shared<PhysicsObject>(gameObjects[lastIndex]->GetTransform(), PhysicsLayers::Trigger, 1.0f, false, false));
	gameObjects[lastIndex]->SetName("Skybox");

    
    
    

	

	

    for (size_t i = 0; i < gameObjects.size(); i++) {
        gameObjects[i]->Spawn();
    }

	
}

void GameManager::Update()
{
    // MshMngr->ClearRenderList();
    // MeshManager::GetInstance()->DrawWireCube(glm::vec3(1.0f, 2.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    //Rotate Camera
    //  Hold to unlock camera 

	//these following 4 statements keep the cube locked to not be 
	//able to go off of the screen for now
	if (gameObjects[0]->GetTransform()->GetPosition().y > 2.0f) {
		gameObjects[0]->GetTransform()->SetPosition(glm::vec3(gameObjects[0]->GetTransform()->GetPosition().x, 2.0f, 0.0f));
	}
	if (gameObjects[0]->GetTransform()->GetPosition().y < -2.0f) {
		gameObjects[0]->GetTransform()->SetPosition(glm::vec3(gameObjects[0]->GetTransform()->GetPosition().x, -2.0f, 0.0f));
	}
	
	if (gameObjects[0]->GetTransform()->GetPosition().x > 2.0f) {
		gameObjects[0]->GetTransform()->SetPosition(glm::vec3(2.0f, gameObjects[0]->GetTransform()->GetPosition().y, 0.0f));
	}
	if (gameObjects[0]->GetTransform()->GetPosition().x < -2.0f) {
		gameObjects[0]->GetTransform()->SetPosition(glm::vec3(-2.0f, gameObjects[0]->GetTransform()->GetPosition().y, 0.0f));
	}

	
	
	//hold and drag the screen to move the camera
    if (InputManager::GetInstance()->GetKey(Controls::RightClick)) {
        lockCamera = !lockCamera; 
	}
	else {
		lockCamera = true;
	}
    if (InputManager::GetInstance()->GetKey(Controls::LeftClick)) {
        lockCamera = !lockCamera;
	}
	else {
		lockCamera = true;
	}

    //  Rotate camera if not locked
    if (!lockCamera) {
        glm::vec2 deltaMouse = InputManager::GetInstance()->GetDeltaMouse();
        if (deltaMouse.x != 0 || deltaMouse.y != 0) {
            deltaMouse = glm::normalize(deltaMouse) / 4.0f;
        }

        glm::quat orientation = Camera::GetMainCamera()->GetTransform()->GetOrientation();
        glm::vec3 rotation = orientation * glm::vec3(deltaMouse.y, 0.0f, 0.0f) + glm::vec3(0.0f, -deltaMouse.x, 0.0f);

        Camera::GetMainCamera()->GetTransform()->Rotate(rotation);
    }

    //Move Cube
    glm::vec3 moveDirection = glm::vec3(0.0f, 0.0f, 0.0f);

	//switch to up and down for now
	//Using wasd will move the cube in the corresponding directions
	//w-up, s-down, a-left, d-right
    if (InputManager::GetInstance()->GetKey(Controls::Forward)) {
        moveDirection += glm::vec3(0.0f, 0.0f, 1.0f);
    }
    if (InputManager::GetInstance()->GetKey(Controls::Back)) {
        moveDirection += glm::vec3(0.0f, 0.0f, -1.0f);
    }
	
    if (InputManager::GetInstance()->GetKey(Controls::Up)) {
        moveDirection += glm::vec3(0.0f, 1.0f, 0.0f);
    }
    if (InputManager::GetInstance()->GetKey(Controls::Down)) {
        moveDirection += glm::vec3(0.0f, -1.0f, 0.0f);
    }
    if (InputManager::GetInstance()->GetKey(Controls::Left)) {
        moveDirection += glm::vec3(-1.0f, 0.0f, 0.0f);
    }
    if (InputManager::GetInstance()->GetKey(Controls::Right)) {
        moveDirection += glm::vec3(1.0f, 0.0f, 0.0f);
    }

    if (moveDirection.x != 0 || moveDirection.y != 0 || moveDirection.z != 0) {
        moveDirection = glm::normalize(moveDirection);
    }

    Camera::GetMainCamera()->GetTransform()->Translate(moveDirection * cameraSpeed * Time::GetDeltaTime(), true);

	//Move cube
	//gameObjects[0]->GetTransform()->Translate(moveDirection * cameraSpeed * Time::GetDeltaTime(), true);

    //Update Lights
    float scaledTime = Time::GetTotalTime() / 2.5f;
    lights[0]->position = glm::vec3(0.0f, 1.1f, 0.0f) + glm::vec3(cos(scaledTime), 0.0f, sin(scaledTime)) * 1.5f;
    
    //Update Game Objects
    for (size_t i = 0; i < gameObjects.size(); i++) {
        gameObjects[i]->Update();
    }

    if (InputManager::GetInstance()->GetKeyPressed(Controls::Jump)) {
        gameObjects[2]->GetPhysicsObject()->ApplyForce(glm::vec3(0.0f, 5000.0f, 0.0f));

        //Spawn Object Sample Code:
        /*
        std::shared_ptr<GameObject> newObject = std::make_shared<GameObject>(EntityManager::GetInstance()->GetMeshes()[MeshTypes::Sphere]);
        gameObjects.push_back(newObject);

        newObject->SetTransform(std::make_shared<Transform>(glm::vec3(0.0f, 2.5f, 0.0f)));
        newObject->SetPhysicsObject(std::make_shared<PhysicsObject>(newObject->GetTransform(), PhysicsLayers::Static, 1.0f, false, true));

        newObject->Spawn();
        */
    }
}

#pragma endregion