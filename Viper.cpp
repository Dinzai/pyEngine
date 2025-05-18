#include <SFML/Graphics.hpp>
#include <Python.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <map>
#include <vector>


#include <fstream>

#include <iostream>

#include "Split.hpp"

struct GridChunk
{

	GridChunk()
	{
		gridChunk.setFillColor(sf::Color::Transparent);
		gridChunk.setSize(sf::Vector2f(30, 30));
		gridChunk.setOutlineColor(sf::Color::White);
		gridChunk.setOutlineThickness(0.2);
	}

	void SetPosition(float x, float y)
	{
		gridChunk.setPosition(sf::Vector2f(x, y));
	}

	void Drawable(sf::RenderWindow& window)
	{
		window.draw(gridChunk);
	}

	sf::RectangleShape gridChunk;

};


struct WorldGrid
{

	WorldGrid()
	{

	}


	void SetGrid(int worldWidth, int worldHeight, int offset)
	{
		for (int i = 0; i < worldWidth; i++)
		{
			for (int j = 0; j < worldHeight; j++)
			{
				GridChunk* chunk = new GridChunk();
				chunk->SetPosition(i * chunk->gridChunk.getSize().x, j * chunk->gridChunk.getSize().y);
				chunks.push_back(chunk);
			}
		}
	}


	void Drawable(sf::RenderWindow& window)
	{
		for (GridChunk* c : chunks)
		{
			c->Drawable(window);
		}
	}

	std::vector<GridChunk*> chunks;

};


class TextureImage
{

public:

	TextureImage()
	{

	}

	sf::CircleShape ReturnCircleShapeTexture(sf::CircleShape& shape, std::string fileLocation)
	{
		texture.loadFromFile(fileLocation);
		shape.setTexture(&texture);
		return shape;
	}

	sf::RectangleShape ReturnRectShapeTexture(sf::RectangleShape& shape, std::string fileLocation)
	{
		texture.loadFromFile(fileLocation);
		shape.setTexture(&texture);
		return shape;
	}

	sf::Texture texture;

};

class PShader
{

public:

	PShader()
	{

	}

	bool SetShader()
	{
		hasShader = true;

		shader.loadFromFile("shader.vert", "shader.frag");

		shader.setUniform("texture", sf::Shader::CurrentTexture);

		shader.setUniform("noise", sf::Shader::CurrentTexture);

		return hasShader;

	}

	sf::Shader* PassShader(sf::Shader* temp)
	{
		shader.bind(temp);
		return &shader;
	}

	sf::Shader shader;
	bool hasShader = false;

};

class Camera
{
public:

	Camera()
	{

	}

	void LockCamera(sf::RenderWindow& window, sf::CircleShape shape)
	{

		camera.setSize(window.getSize().x, window.getSize().y);
		camera.setCenter(shape.getPosition());
		window.setView(camera);

	}

	sf::View camera;

};

class Controller
{

public:


	Controller()
	{

	}

	void FixedUpdate(sf::Event& event)
	{
		if (event.type == sf::Event::KeyPressed)
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			{
				isJumping = true;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			{

			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			{
				isMovingLeft = true;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			{
				isMovingRight = true;
			}
		}

		if (event.type == sf::Event::KeyReleased)
		{
			if (event.key.code == sf::Keyboard::A)
			{
				isMovingLeft = false;
			}

			if (event.key.code == sf::Keyboard::D)
			{
				isMovingRight = false;
			}

			if (event.key.code == sf::Keyboard::W)
			{
				isJumping = false;
				
			}

		}


	}


	bool isMovingRight = false;
	bool isMovingLeft = false;
	float speed = 200;
	float jumpSpeed = 150;
	bool isJumping = false;
	bool previous = false;

};


class Phys_Vector
{
public:

	Phys_Vector(float x, float y)
	{
		mass = 0;
		position = sf::Vector2f(x, y);
		velocity = sf::Vector2f(0, 0);
		acceleration = sf::Vector2f(0, 0);
		isGrounded = false;
		force = sf::Vector2f(0, 0);
	}

	float mass = 0;
	sf::Vector2f position;
	sf::Vector2f  velocity;
	sf::Vector2f  acceleration;
	sf::Vector2f  gravity = sf::Vector2f(0, 98);
	bool isGrounded = false;
	sf::Vector2f force;

};



class PhysicsObject : public Phys_Vector
{

public:


	Controller* c;
	Camera* cam;
	TextureImage* tI;
	PShader* pS;

	PhysicsObject(float x, float y, float radius, float mass, sf::Color color, int index) : Phys_Vector(x, y)
	{

		position = sf::Vector2f(x, y);
		this->mass = mass;
		this->radius = radius;
		this->index = index;

		shape.setFillColor(color);
		shape.setPosition(position);
		shape.setOrigin(radius, radius);
		shape.setRadius(radius);

		hasController = false;
		hasCamera = false;
		c = nullptr;
		cam = nullptr;
		this->isRect = false;

	}


	PhysicsObject(float x, float y, int index) : Phys_Vector(x, y)//only ground objects
	{
		this->width = 30;
		this->height = 30;
		this->mass = 1000;
		this->index = index;

		otherShape.setPosition(sf::Vector2f(x, y));
		otherShape.setFillColor(sf::Color::Blue);
		otherShape.setOrigin(this->width * 0.5, this->height * 0.5);
		otherShape.setSize(sf::Vector2f(width, height));

		hasController = false;
		hasCamera = false;
		c = nullptr;
		cam = nullptr;
		isGrounded = true;
		this->isRect = true;
	}

	virtual ~PhysicsObject()
	{
	}


	void PhysCalc()
	{
		acceleration.y = gravity.y;
	}


	bool OutOfBoundsY(sf::RenderWindow& window)
	{

		if (position.y > window.getSize().y - radius)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	/*
	bool CollisionRect(sf::RectangleShape& rect, PhysicsObject& circleObj)
	{

		sf::Vector2f rectPos = rect.getPosition();
		sf::Vector2f rectHalf = rect.getSize() / 2.f;

		sf::Vector2f circleCenter = circleObj.position + sf::Vector2f(circleObj.radius, circleObj.radius);

		float radius = circleObj.radius;

		float left = rectPos.x - rectHalf.x;
		float right = rectPos.x + rectHalf.x;
		float top = rectPos.y - rectHalf.y;
		float bottom = rectPos.y + rectHalf.y;

		float closestX = circleCenter.x;
		if (circleCenter.x < left) closestX = left;
		else if (circleCenter.x > right) closestX = right;

		float closestY = circleCenter.y;
		if (circleCenter.y < top) closestY = top;
		else if (circleCenter.y > bottom) closestY = bottom;

		float dx = closestX - circleCenter.x;
		float dy = closestY - circleCenter.y;
		float distanceSq = dx * dx + dy * dy;

		if (distanceSq <= radius * radius)
		{

			if (circleCenter.y + radius > top && circleCenter.y < top)
			{

				float overlap = (circleCenter.y + radius) - top;

				circleObj.position.y -= overlap;

				circleObj.velocity.y = 0.f;

				circleObj.isGrounded = true;
				circleObj.c->previous = false;

				return true;
			}
		}
		else
		{
			//circleObj.isGrounded = false;
		}

		return false;  // No collision
	}
	*/
	
	bool CollisionRect(sf::RectangleShape& rect, PhysicsObject& circleObj)
	{
		sf::Vector2f rectPos = rect.getPosition();
		sf::Vector2f rectHalf = rect.getSize() / 2.f;

		sf::Vector2f circleCenter = circleObj.position + sf::Vector2f(circleObj.radius, circleObj.radius);

		float radius = circleObj.radius;

		float left = rectPos.x - rectHalf.x;
		float right = rectPos.x + rectHalf.x;
		float top = rectPos.y - rectHalf.y;
		float bottom = rectPos.y + rectHalf.y;

		float closestX = circleCenter.x;
		if (circleCenter.x < left) closestX = left;
		else if (circleCenter.x > right) closestX = right;

		float closestY = circleCenter.y;
		if (circleCenter.y < top) closestY = top;
		else if (circleCenter.y > bottom) closestY = bottom;

		float dx = closestX - circleCenter.x;
		float dy = closestY - circleCenter.y;
		float distanceSq = dx * dx + dy * dy;

		if (distanceSq <= radius * radius)
		{
			float overlapX = radius - std::abs(dx);
			float overlapY = radius - std::abs(dy);

			if (overlapX < overlapY)
			{
				// Horizontal collision (left/right)
				if (dx > 0) // Collision from left
				{
					circleObj.position.x -= overlapX;
				}
				else // Collision from right
				{
					circleObj.position.x += overlapX;
				}
				circleObj.velocity.x = 0.f;
				return true;
			}
			// Vertical collision 
			if (circleCenter.y + radius > top && circleCenter.y < top)
			{

				float overlap = (circleCenter.y + radius) - top;

				circleObj.position.y -= overlap;

				circleObj.velocity.y = 0;

				circleObj.isGrounded = true;
				circleObj.c->previous = false;

				return true;
			}

			if (circleCenter.y + radius > bottom && circleCenter.y < bottom)
			{
				float overlap = (circleCenter.y + radius) - bottom;
				std::cout << "Overlap value" << overlap <<'\n';
				circleObj.velocity.y = 0;
				circleObj.position.y += overlap;
				return true;
			}
			
		}

		return false;
	}

	bool Collision(PhysicsObject& self, PhysicsObject& target)
	{

		if (this->isRect)
		{
			return CollisionRect(otherShape, target);
		}
		if (target.isRect)
		{
			return CollisionRect(target.otherShape, self);
		}


		float dx = self.position.x - target.position.x;
		float dy = self.position.y - target.position.y;
		float distance = sqrt((dx * dx) + (dy * dy));

		if (distance <= self.radius + target.radius)
		{
			return true;

		}
		else
		{
			return false;
		}

	}

	sf::Vector2f Normalize(PhysicsObject& self, PhysicsObject& target)
	{
		sf::Vector2f vector2D;

		float dx = target.position.x - self.position.x;
		float dy = target.position.y - self.position.y;
		float distance = sqrt((dx * dx) + (dy * dy));
		if (distance == 0)
		{
			vector2D = sf::Vector2f(0, 0);
		}
		else
		{
			vector2D = sf::Vector2f(dx / distance, dy / distance);
		}

		return vector2D;

	}

	std::vector<sf::Vector2f> Phys_Collision_Calc(PhysicsObject& self, PhysicsObject& target)
	{

		velocities.clear();

		sf::Vector2f normalized = Normalize(self, target);

		float dot1 = self.velocity.x * normalized.x + self.velocity.y * normalized.y;
		float dot2 = target.velocity.x * normalized.x + target.velocity.y * normalized.y;

		sf::Vector2f v1Norm = sf::Vector2f(dot1 * normalized.x, dot1 * normalized.y);
		sf::Vector2f v2Norm = sf::Vector2f(dot2 * normalized.x, dot2 * normalized.y);

		sf::Vector2f newV1 = sf::Vector2f(
			(v1Norm.x * (self.mass - target.mass) + 2 * target.mass * v2Norm.x) / (self.mass + target.mass),
			(v1Norm.y * (self.mass - target.mass) + 2 * target.mass * v2Norm.y) / (self.mass + target.mass));


		sf::Vector2f newV2 = sf::Vector2f(
			(v2Norm.x * (target.mass - self.mass) + 2 * self.mass * v1Norm.x) / (self.mass + target.mass),
			(v2Norm.y * (target.mass - self.mass) + 2 * self.mass * v1Norm.y) / (self.mass + target.mass));



		velocities.push_back(newV1);
		velocities.push_back(newV2);


		return velocities;

	}

	void FixedUpdate(sf::Event& event)
	{
		if (hasController)
		{
			c->FixedUpdate(event);
		}

	}

	void Update(sf::Time deltaTime, sf::RenderWindow& window)
	{

		if (this->isGrounded)
		{
			acceleration.y = 0;
			velocity.y = 0;

		}
		PhysCalc();

		if (this->hasController && this->index == 0)
		{
			this->velocity.x = c->speed;

			if (c->isJumping && !c->previous)
			{
				this->velocity.y = -c->jumpSpeed;
				this->position.y += this->velocity.y * deltaTime.asSeconds();
				this->isGrounded = false;
				c->previous = true;
			}

			if (c->isMovingRight)
			{
				this->position.x += this->velocity.x * deltaTime.asSeconds();
			}
			else if (c->isMovingLeft)
			{
				this->position.x -= this->velocity.x * deltaTime.asSeconds();
			}

			this->velocity.y += this->acceleration.y * deltaTime.asSeconds();
			this->position.y += this->velocity.y * deltaTime.asSeconds();

		}
		else
		{
			velocity.x += acceleration.x * deltaTime.asSeconds();
			position.x += velocity.x * deltaTime.asSeconds();

			velocity.y += acceleration.y * deltaTime.asSeconds();
			position.y += velocity.y * deltaTime.asSeconds();
		}

		shape.setPosition(position);
	}


	void Drawable(sf::RenderWindow& window)
	{

		if (hasCamera && index == 0)
		{
			cam->LockCamera(window, shape);
		}

		if (hasShader)
		{
			window.draw(shape, shader);
			window.draw(otherShape, shader);
		}
		else
		{
			window.draw(shape);
			window.draw(otherShape);
		}

	}


	std::vector<sf::Vector2f> velocities;

	sf::Shader* shader;

	sf::CircleShape shape;
	sf::RectangleShape otherShape;

	float radius = 0;
	int index;

	float width = 0;
	float height = 0;

	bool isRect = false;

	bool hasCamera = false;
	bool hasController = false;
	bool hasShader = false;

};

class Bucket
{

public:

	Bucket()
	{

	}

	void AddObject(PhysicsObject* temp)
	{

		allPhysObjects.push_back(temp);
	}


	void RemoveObject()
	{

		allPhysObjects.pop_back();
	}


	void FixedUpdate(sf::Event& event)
	{
		for (PhysicsObject* obj : allPhysObjects)
		{
			obj->c->FixedUpdate(event);
		}
	}

	void Update(sf::Time deltaTime, sf::RenderWindow& window)
	{
		for (PhysicsObject* obj : allPhysObjects)
		{
			obj->Update(deltaTime, window);
		}
	}

	void GroupCollision(sf::RenderWindow& window)
	{
		for (int i = 0; i < allPhysObjects.size(); i++)
		{
			PhysicsObject& tempA = *allPhysObjects[i];
			bool groundedThisFrame = false;

			for (int j = i + 1; j < allPhysObjects.size(); j++)
			{
				PhysicsObject& tempB = *allPhysObjects[j];

				// Ball-on-Ball Collision
				if (tempA.Collision(tempA, tempB) && !tempB.isRect)
				{

					std::vector<sf::Vector2f> resultVelocities = tempA.Phys_Collision_Calc(tempA, tempB);
					if (resultVelocities.size() < 2) continue;

					tempA.velocity = resultVelocities[0];
					tempB.velocity = resultVelocities[1];

					// Calculate separation
					float dx = tempA.position.x - tempB.position.x;
					float dy = tempA.position.y - tempB.position.y;
					float distance = sqrt(dx * dx + dy * dy);
					if (distance == 0) distance = 0.01f;

					float overlap = tempA.radius + tempB.radius - distance;
					float nx = dx / distance;
					float ny = dy / distance;

					// Apply separation
					tempA.position.x += nx * overlap * (tempB.mass / (tempA.mass + tempB.mass));
					tempA.position.y += ny * overlap * (tempB.mass / (tempA.mass + tempB.mass));
					tempB.position.x -= nx * overlap * (tempA.mass / (tempA.mass + tempB.mass));
					tempB.position.y -= ny * overlap * (tempA.mass / (tempA.mass + tempB.mass));

					// Dampen horizontal movement
					tempA.velocity.x *= 0.95f;
					tempB.velocity.x *= 0.95f;
				}
				// Ball-on-Rect Collision (platform)
				else if (tempB.isRect && tempA.Collision(tempA, tempB))
				{
					// Improved ground collision check
					bool falling = tempA.velocity.y >= 0.f;
					bool abovePlatform = (tempA.position.y + tempA.radius) <= (tempB.position.y + tempB.otherShape.getSize().y / 2.f);
					
					if (falling && abovePlatform)
					{
						tempA.velocity.y = 0.f;
						tempA.position.y = tempB.position.y - tempA.radius;
						groundedThisFrame = true;
						
						tempA.velocity.x *= 0.95f;
					}
				}
			}

			if (!tempA.isRect)
			{
				tempA.isGrounded = groundedThisFrame;

				if (groundedThisFrame && tempA.hasController && tempA.index == 0)
				{
					
					tempA.c->previous = false;
				}
			}
		}
	}

	void Drawable(sf::RenderWindow& window)
	{
		for (PhysicsObject* obj : allPhysObjects)
		{
			obj->Drawable(window);
		}
	}

	std::vector<sf::Vector2f> listOfVelocites;

	std::vector<PhysicsObject*> allPhysObjects;
};



PhysicsObject* AddComponenetType(PhysicsObject* theObj, std::string type)
{

	if (type == "controller")
	{

		theObj->c = new Controller();
		theObj->hasController = true;

	}

	if (type == "camera")
	{

		theObj->cam = new Camera();
		theObj->hasCamera = true;

	}

	return theObj;

}

PhysicsObject* AddShader(PhysicsObject* theObj, std::string type)
{


	if (type == "shader")
	{
		theObj->pS = new PShader();
		theObj->hasShader = theObj->pS->SetShader();
		theObj->shader = theObj->pS->PassShader(theObj->shader);

	}

	return theObj;
}

PhysicsObject* AddTexture(PhysicsObject* theObj, std::string type, std::string location)
{

	if (type == "texture")
	{

		theObj->tI = new TextureImage();
		theObj->shape.setFillColor(sf::Color::White);
		theObj->shape = theObj->tI->ReturnCircleShapeTexture(theObj->shape, location);

	}

	return theObj;

}


//--------------------------------------------------


struct Object // from python
{

public:

	float xPosition;
	float yPosition;
	float radius;
	float mass;

};


Bucket allObj;



struct Level
{

	Level(std::string fileLocation)
	{
		AddLevel(fileLocation);
	}

	void AddLevel(std::string fileLocation)
	{
		splitSlash.Do(fileLocation, ',');		
		level = splitSlash.ConverToInt(splitSlash.goal);
		
	}

	void LoadLevel(int worldWidth, float offset)
	{
		for (int i = 0; i < level.size(); i++)
		{
			if (level[i] == 1)
			{
				int x = i % worldWidth;
				int y = i / worldWidth;

				PhysicsObject* obj = new PhysicsObject(x * obj->otherShape.getSize().x + offset, y * obj->otherShape.getSize().y + offset, index);

				PhysicsObject* newObj = AddComponenetType(obj, "controller");

				allObj.AddObject(newObj);

				index++;

			}

		}
	}

	int index = 1;
	std::vector<int> level;
	Dinzai::HelperFunctions splitSlash;

};


pybind11::object Spawn(const std::map<std::string, pybind11::object>& command)
{
	auto position = command.at("position").cast<std::vector<float>>();
	float mass = command.at("mass").cast<float>();
	float radius = command.at("radius").cast<float>();

	float red = command.at("red").cast<float>();
	float green = command.at("green").cast<float>();
	float blue = command.at("blue").cast<float>();
	float alpha = command.at("alpha").cast<float>();

	int index = command.at("index").cast<int>();

	sf::Color color = sf::Color(red, green, blue, alpha);

	Object obj{ position[0], position[1], radius, mass };

	PhysicsObject* theObject = new PhysicsObject(obj.xPosition, obj.yPosition, obj.radius, obj.mass, color, index);

	allObj.AddObject(theObject);

	return pybind11::cast(allObj.allPhysObjects[index], pybind11::return_value_policy::reference);

}


pybind11::object AddComponent(pybind11::object obj, const std::map<std::string, pybind11::object>& component)
{
	PhysicsObject* theObj = obj.cast<PhysicsObject*>();

	allObj.allPhysObjects.pop_back();

	std::string componenetType = component.at("controller").cast<std::string>();

	PhysicsObject* addedObj = AddComponenetType(theObj, componenetType);

	int index = addedObj->index;

	allObj.AddObject(addedObj);

	return pybind11::cast(allObj.allPhysObjects[index], pybind11::return_value_policy::reference);

}


pybind11::object AddCamera(pybind11::object obj, const std::map<std::string, pybind11::object>& component)
{
	PhysicsObject* theObj = obj.cast<PhysicsObject*>();

	allObj.allPhysObjects.pop_back();

	std::string componenetType = component.at("camera").cast<std::string>();

	PhysicsObject* addedObj = AddComponenetType(theObj, componenetType);

	int index = addedObj->index;

	allObj.AddObject(addedObj);

	return pybind11::cast(allObj.allPhysObjects[index], pybind11::return_value_policy::reference);

}

pybind11::object Shader(pybind11::object obj, const std::map<std::string, pybind11::object>& command)
{
	PhysicsObject* theObj = obj.cast<PhysicsObject*>();

	allObj.allPhysObjects.pop_back();

	std::string componenetType = command.at("shader").cast<std::string>();

	PhysicsObject* addedObj = AddShader(theObj, componenetType);

	int index = theObj->index;

	allObj.AddObject(addedObj);

	return pybind11::cast(allObj.allPhysObjects[index], pybind11::return_value_policy::reference);

}


pybind11::object Texture(pybind11::object obj, const std::map<std::string, pybind11::object>& command, const std::map<std::string, pybind11::object>& file)
{
	PhysicsObject* theObj = obj.cast<PhysicsObject*>();

	allObj.allPhysObjects.pop_back();

	std::string componenetType = command.at("texture").cast<std::string>();

	std::string location = file.at("location").cast<std::string>();

	PhysicsObject* addedObj = AddTexture(theObj, componenetType, location);

	int index = theObj->index;

	allObj.AddObject(addedObj);

	return pybind11::cast(allObj.allPhysObjects[index], pybind11::return_value_policy::reference);

}



void Run()
{

	Level* l = new Level("data.txt");

	l->LoadLevel(20, 300);

	sf::RenderWindow window(sf::VideoMode(1280, 720), "Viper Engine !");
	sf::Time deltaTime;
	sf::Clock mainClock;

	
	

	while (window.isOpen())
	{

		deltaTime = mainClock.getElapsedTime();

		allObj.Update(deltaTime, window);
		allObj.GroupCollision(window);

		sf::Event event;
		while (window.pollEvent(event))
		{

			if (event.type == sf::Event::Closed)
			{
				window.close();
			}

			allObj.FixedUpdate(event);

		}

		mainClock.restart();

		window.clear();

		allObj.Drawable(window);

		window.display();

	}

}

PYBIND11_MODULE(Viper, m)
{


	pybind11::class_<PhysicsObject>(m, "PhysicsObject")
		.def_readwrite("position", &PhysicsObject::position)
		.def_readwrite("radius", &PhysicsObject::radius)
		.def_readwrite("mass", &PhysicsObject::mass)
		.def_readwrite("index", &PhysicsObject::index)
		.def_readwrite("hasController", &PhysicsObject::hasController)
		.def_readwrite("hasShader", &PhysicsObject::hasShader);

	m.def("Spawn", &Spawn);
	m.def("AddComponent", &AddComponent);
	m.def("AddCamera", &AddCamera);
	m.def("AddShader", &Shader);
	m.def("AddTexture", &Texture);
	m.def("Run", &Run);
}
