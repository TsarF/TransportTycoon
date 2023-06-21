#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <string>

// Size of the terrain
const int MAP_SIZE = 256;

const int permutation[] = {151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
						   140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
						   247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
						   57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
						   74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
						   60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
						   65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
						   200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
						   52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
						   207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
						   119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
						   129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
						   218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
						   81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
						   184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
						   222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180};

// Random number generator
std::mt19937 rng;

// Generate random float between min and max
float randomFloat(float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(rng);
}

// Linear interpolation
float lerp(float a, float b, float x)
{
	return a + x * (b - a);
}

// Gradient calculation
float grad(int hash, float x, float y)
{
	int h = hash & 15;
	float u = h < 8 ? x : y;
	float v = h < 4 ? y : (h == 12 || h == 14 ? x : 0);
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

// Generate Perlin noise
float perlin(float x, float y)
{
	int xi = static_cast<int>(x) & 255;
	int yi = static_cast<int>(y) & 255;
	float xf = x - static_cast<int>(x);
	float yf = y - static_cast<int>(y);

	float u = xf * xf * xf * (xf * (xf * 6 - 15) + 10);
	float v = yf * yf * yf * (yf * (yf * 6 - 15) + 10);

	int a = permutation[xi] + yi;
	int aa = permutation[a];
	int ab = permutation[a + 1];
	int b = permutation[xi + 1] + yi;
	int ba = permutation[b];
	int bb = permutation[b + 1];

	float x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1, yf), u);
	float x2 = lerp(grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1), u);

	return lerp(x1, x2, v);
}

// Generate terrain heightmap using Perlin noise
std::vector<std::vector<float>> generateTerrain()
{
	std::vector<std::vector<float>> heightmap(MAP_SIZE, std::vector<float>(MAP_SIZE, 0.0f));

	// Generate random permutation table for Perlin noise
	std::vector<int> perm;
	perm.resize(512);
	for (int i = 0; i < 256; ++i)
	{
		perm[i] = i;
	}

	std::shuffle(perm.begin(), perm.begin() + 256, rng);
	for (int i = 0; i < 256; ++i)
	{
		perm[i + 256] = perm[i];
	}
	float min = 0.0f;

	for (int x = 0; x < MAP_SIZE; ++x)
	{
		for (int y = 0; y < MAP_SIZE; ++y)
		{
			float frequency = 0.01f;
			float amplitude = 1.0f;
			float noise = 0.0f;

			for (int i = 0; i < 1; i++)
			{
				noise += (perlin(x * frequency, y * frequency)) * amplitude;
				frequency *= 2.0f;
				amplitude *= 0.5f;
			}

			heightmap[x][y] = noise;
			if (heightmap[x][y] < min) min = heightmap[x][y];
		}
	}

	for (int x = 0; x < MAP_SIZE; ++x)
	{
		for (int y = 0; y < MAP_SIZE; ++y)
		{
			heightmap[x][y] -= min;
		}
	}

	return heightmap;
}

const float gaussian_stamp_K5[5][5] =
	{
		{0.00296902, 0.0133062, 0.0219382, 0.0133062, 0.00296902},
		{0.0133062, 0.0596343, 0.0983203, 0.0596343, 0.0133062},
		{0.0219382, 0.0983203, 0.162103, 0.0983203, 0.0219382},
		{0.0133062, 0.0596343, 0.0983203, 0.0596343, 0.0133062},
		{0.00296902, 0.0133062, 0.0219382, 0.0133062, 0.00296902}};

struct HeightAndGradient
{
public:
	float height;
	float gradientX;
	float gradientY;
};

float gaussianArraySubtract(std::vector<std::vector<float>> &a, float **b, int posX, int posY, int mapSize, float volume)
{
	float carrySediment = 0.0f;
	for (int x = -2; x < 3; x++)
	{
		for (int y = -2; y < 3; y++)
		{
			int erodePosX = x + posX;
			int erodePosY = y + posY;
			if (erodePosX < mapSize && erodePosY < mapSize && erodePosX >= 0 && erodePosY >= 0)
			{
				float erosionWeight = gaussian_stamp_K5[x + 2][y + 2] * volume;
				float amountToErode = (a[erodePosX][erodePosY] < erosionWeight) ? a[erodePosX][erodePosY] : erosionWeight;

				carrySediment += amountToErode;

				a[erodePosX][erodePosY] -= amountToErode;
			}
		}
	}
	return carrySediment;
}

void gaussianArrayAdd(std::vector<std::vector<float>> &a, float **b, int posX, int posY, int mapSize, float volume)
{
	for (int x = -2; x < 3; x++)
	{
		for (int y = -2; y < 3; y++)
		{
			int erodePosX = x + posX;
			int erodePosY = y + posY;
			if (erodePosX < mapSize && erodePosY < mapSize && erodePosX >= 0 && erodePosY >= 0)
			{
				float erosionWeight = gaussian_stamp_K5[x + 2][y + 2] * volume;

				a[erodePosX][erodePosY] += erosionWeight;
			}
		}
	}
}

HeightAndGradient CalculateHeightAndGradient(std::vector<std::vector<float>> &nodes, int mapSize, float posX, float posY)
{
	int coordX = (int)posX;
	int coordY = (int)posY;

	// Calculate droplet's offset inside the cell (0,0) = at NW node, (1,1) = at SE node
	float x = posX - coordX;
	float y = posY - coordY;

	// Calculate heights of the four nodes of the droplet's cell

	float heightNW = nodes[posX][posY];
	float heightNE = nodes[posX+1][posY];
	float heightSW = nodes[posX][posY+1];
	float heightSE = nodes[posX+1][posY+1];

	/*
	float heightNW = nodes[posX-1][posY+1];
	float heightNE = nodes[posX-1][posY-1];
	float heightSW = nodes[posX+1][posY-1];
	float heightSE = nodes[posX+1][posY+1];*/

	// Calculate droplet's direction of flow with bilinear interpolation of height difference along the edges
	float gradientX = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
	float gradientY = (heightSW - heightNW) * (1 - x) + (heightSE - heightNE) * x;

	// Calculate height with bilinear interpolation of the heights of the nodes of the cell
	float height = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) + heightSW * (1 - x) * y + heightSE * x * y;

	if (isnan(gradientX) || isnan(gradientY))
	{
		printf("balls");
	}

	return {height, gradientX, gradientY};
}

void simulateErosion(std::vector<std::vector<float>> &heightmap)
{
	for (int iteration = 0; iteration < 1000000; iteration++)
	{
		// Create water droplet at random point on map
		float posX = randomFloat(0, MAP_SIZE - 1);
		float posY = randomFloat(0, MAP_SIZE - 1);
		float dirX = 0;
		float dirY = 0;
		float speed = 1.0f;
		float water = 1.0f;
		float sediment = 0;
		float inertia = 0.05f;
		int maxLifetime = 30;
		float sedimentCapacityFactor = 4.0f;
		float minSedimentCapacity = 0.01f;
		float depositSpeed = 0.3f;
		float erodeSpeed = 0.3f;
		float evaporateSpeed = 0.01f;

		for (int lifetime = 0; lifetime < maxLifetime; lifetime++)
		{
			int nodeX = (int)posX;
			int nodeY = (int)posY;
			int dropletIndex = nodeY * MAP_SIZE + nodeX;
			// Calculate droplet's offset inside the cell (0,0) = at NW node, (1,1) = at SE node
			float cellOffsetX = posX - nodeX;
			float cellOffsetY = posY - nodeY;

			// Calculate droplet's height and direction of flow with bilinear interpolation of surrounding heights
			HeightAndGradient heightAndGradient = CalculateHeightAndGradient(heightmap, MAP_SIZE, posX, posY);

			// Update the droplet's direction and position (move position 1 unit regardless of speed)
			dirX = (dirX * inertia - heightAndGradient.gradientX * (1 - inertia));
			dirY = (dirY * inertia - heightAndGradient.gradientY * (1 - inertia));
			// Normalize direction
			float len = sqrtf(dirX * dirX + dirY * dirY);
			if (len != 0.0f)
			{
				dirX /= len;
				dirY /= len;
			}
			posX += dirX;
			posY += dirY;

			// Stop simulating droplet if it's not moving or has flowed over edge of map
			if ((dirX == 0 && dirY == 0) || posX < 0 || posX >= MAP_SIZE - 1 || posY < 0 || posY >= MAP_SIZE - 1)
			{
				break;
			}

			// Find the droplet's new height and calculate the deltaHeight
			float newHeight = CalculateHeightAndGradient(heightmap, MAP_SIZE, posX, posY).height;
			float deltaHeight = newHeight - heightAndGradient.height;
			// delete heightAndGradient;

			// Calculate the droplet's sediment capacity (higher when moving fast down a slope and contains lots of water)
			float sedimentCapacity = std::max(-deltaHeight * speed * water * sedimentCapacityFactor, minSedimentCapacity);

			// If carrying more sediment than capacity, or if flowing uphill:
			if (sediment > sedimentCapacity || deltaHeight > 0)
			{
				// If moving uphill (deltaHeight > 0) try fill up to the current height, otherwise deposit a fraction of the excess sediment
				float amountToDeposit = (deltaHeight > 0) ? std::min(deltaHeight, sediment) : (sediment - sedimentCapacity) * depositSpeed;
				sediment -= amountToDeposit;

				// Add the sediment to the four nodes of the current cell using bilinear interpolation
				// Deposition is not distributed over a radius (like erosion) so that it can fill small pits
				gaussianArrayAdd(heightmap, (float **)gaussian_stamp_K5, nodeX, nodeY, MAP_SIZE, amountToDeposit);
				// heightmap[nodeX][nodeY] += amountToDeposit;// *(1 - cellOffsetX)* (1 - cellOffsetY);
				// heightmap[nodeX+1][nodeY] 	+=amountToDeposit * cellOffsetX * (1 - cellOffsetY);
				// heightmap[nodeX][nodeY+1] 	+=amountToDeposit * (1 - cellOffsetX) * cellOffsetY;
				// heightmap[nodeX+1][nodeY+1] +=amountToDeposit * cellOffsetX * cellOffsetY;

				/*
				map[dropletIndex] += amountToDeposit * (1 - cellOffsetX) * (1 - cellOffsetY);
				map[dropletIndex + 1] += amountToDeposit * cellOffsetX * (1 - cellOffsetY);
				map[dropletIndex + mapSize] += amountToDeposit * (1 - cellOffsetX) * cellOffsetY;
				map[dropletIndex + mapSize + 1] += amountToDeposit * cellOffsetX * cellOffsetY;
				*/
			}
			else
			{
				// Erode a fraction of the droplet's current carry capacity.
				// Clamp the erosion to the change in height so that it doesn't dig a hole in the terrain behind the droplet
				float amountToErode = std::min((sedimentCapacity - sediment) * erodeSpeed, -deltaHeight);
				sediment += gaussianArraySubtract(heightmap, (float **)gaussian_stamp_K5, nodeX, nodeY, MAP_SIZE, amountToErode);
				// heightmap[nodeX][nodeY]-=amountToErode;
				// sediment += amountToErode;

				// Use erosion brush to erode from all nodes inside the droplet's erosion radius
				/*
				for (int brushPointIndex = 0; brushPointIndex < erosionBrushIndices[dropletIndex].Length; brushPointIndex++) {
					int nodeIndex = erosionBrushIndices[dropletIndex][brushPointIndex];
					float weighedErodeAmount = amountToErode * erosionBrushWeights[dropletIndex][brushPointIndex];
					float deltaSediment = (map[nodeIndex] < weighedErodeAmount) ? map[nodeIndex] : weighedErodeAmount;
					map[nodeIndex] -= deltaSediment;
					sediment += deltaSediment;
				}
				*/
			}

			// Update droplet's speed and water content
			speed = sqrtf(speed * speed + abs(deltaHeight) * 4.0f); // gravity
			water *= (1 - evaporateSpeed);
		}
	}
}

int main()
{
	// Initialize random number generator
	std::random_device rd;
	rng.seed(rd());

	// Generate terrain heightmap
	std::vector<std::vector<float>> heightmap = generateTerrain();

	std::ofstream before(std::string(PROJECT_PATH) + std::string("/tests/before.map"));
	std::ofstream after(std::string(PROJECT_PATH) + std::string("/tests/after.map"));

	for (int y = 0; y < MAP_SIZE; ++y)
	{
		for (int x = 0; x < MAP_SIZE; ++x)
		{
			std::string str = std::to_string(heightmap[x][y]);
			before.write(str.c_str(), str.length());
			(x != MAP_SIZE - 1) ? before.write(", ", 2) : before.write("\n", 1);
		}
	}
	before.close();

	// Simulate water erosion
	simulateErosion(heightmap);

	for (int y = 0; y < MAP_SIZE; ++y)
	{
		for (int x = 0; x < MAP_SIZE; ++x)
		{
			std::string str = std::to_string(heightmap[x][y]);
			after.write(str.c_str(), str.length());
			(x != MAP_SIZE - 1) ? after.write(", ", 2) : after.write("\n", 1);
		}
	}
	after.close();
	system((std::string("python \"") + std::string(PROJECT_PATH) + std::string("/tests/") + std::string("terrain_to_png.py\" ") + std::string(PROJECT_PATH)).c_str());

	return 0;
}