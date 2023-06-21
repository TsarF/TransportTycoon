#include <vector>
#include <array>

const float gaussian_stamp_K5[5][5] = 
{
	{0.00296902,   0.0133062,    0.0219382,    0.0133062,    0.00296902},
	{0.0133062,    0.0596343,    0.0983203,    0.0596343,    0.0133062},
	{0.0219382,    0.0983203,    0.162103,     0.0983203,    0.0219382},
	{0.0133062,    0.0596343,    0.0983203,    0.0596343,    0.0133062},
	{0.00296902,   0.0133062,    0.0219382,    0.0133062,    0.00296902}
};

struct HeightAndGradient
{
	public:
	float height;
	float gradientX;
	float gradientY;
};

float gaussianArraySubtract(std::vector<std::vector<float>>& a, float** b, int posX, int posY, int mapSize, float volume)
{
	float carrySediment = 0.0f;
	for(int x = -2; x < 3; x++)
	{
		for(int y = -2; y < 3; y++)
		{
			int erodePosX = x + posX;
			int erodePosY = y + posY;
			if(erodePosX < mapSize && erodePosY < mapSize && erodePosX >= 0 && erodePosY >= 0)
			{
				float erosionWeight = b[x+2][y+2] * volume;
				float amountToErode = (a[erodePosX][erodePosY] < erosionWeight) ? a[erodePosX][erodePosY] : erosionWeight;

				carrySediment += amountToErode;

				a[erodePosX][erodePosY]-= amountToErode;
			}
		}
	}
	return carrySediment;
}

HeightAndGradient CalculateHeightAndGradient (std::vector<std::vector<float>>& nodes, int mapSize, float posX, float posY)
{
	int coordX = (int) posX;
	int coordY = (int) posY;

	// Calculate droplet's offset inside the cell (0,0) = at NW node, (1,1) = at SE node
	float x = posX - coordX;
	float y = posY - coordY;

	// Calculate heights of the four nodes of the droplet's cell
	float heightNW = nodes[posX-1][posY+1];
	float heightNE = nodes[posX+1][posY+1];
	float heightSW = nodes[posX-1][posY-1];
	float heightSE = nodes[posX+1][posY-1];

	// Calculate droplet's direction of flow with bilinear interpolation of height difference along the edges
	float gradientX = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
	float gradientY = (heightSW - heightNW) * (1 - x) + (heightSE - heightNE) * x;

	// Calculate height with bilinear interpolation of the heights of the nodes of the cell
	float height = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) + heightSW * (1 - x) * y + heightSE * x * y;

	return {height, gradientX, gradientY };
}

void simulateErosion(std::vector<std::vector<float>>& heightmap)
{
	for (int iteration = 0; iteration < 100; iteration++)
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
			int nodeX = (int) posX;
			int nodeY = (int) posY;
			int dropletIndex = nodeY * MAP_SIZE + nodeX;
			// Calculate droplet's offset inside the cell (0,0) = at NW node, (1,1) = at SE node
			float cellOffsetX = posX - nodeX;
			float cellOffsetY = posY - nodeY;

			// Calculate droplet's height and direction of flow with bilinear interpolation of surrounding heights
			HeightAndGradient heightAndGradient = CalculateHeightAndGradient (heightmap, MAP_SIZE, posX, posY);

			// Update the droplet's direction and position (move position 1 unit regardless of speed)
			dirX = (dirX * inertia - heightAndGradient.gradientX * (1 - inertia));
			dirY = (dirY * inertia - heightAndGradient.gradientY * (1 - inertia));
			// Normalize direction
			float len = sqrtf(dirX * dirX + dirY * dirY);
			if (len != 0) {
				dirX /= len;
				dirY /= len;
			}
			posX += dirX;
			posY += dirY;

			// Stop simulating droplet if it's not moving or has flowed over edge of map
			if ((dirX == 0 && dirY == 0) || posX < 0 || posX >= MAP_SIZE - 1 || posY < 0 || posY >= MAP_SIZE - 1) {
				break;
			}

			// Find the droplet's new height and calculate the deltaHeight
			float newHeight = CalculateHeightAndGradient (heightmap, MAP_SIZE, posX, posY).height;
			float deltaHeight = newHeight - heightAndGradient.height;

			// Calculate the droplet's sediment capacity (higher when moving fast down a slope and contains lots of water)
			float sedimentCapacity = std::max(-deltaHeight * speed * water * sedimentCapacityFactor, minSedimentCapacity);

			// If carrying more sediment than capacity, or if flowing uphill:
			if (sediment > sedimentCapacity || deltaHeight > 0) {
				// If moving uphill (deltaHeight > 0) try fill up to the current height, otherwise deposit a fraction of the excess sediment
				float amountToDeposit = (deltaHeight > 0) ? std::min(deltaHeight, sediment) : (sediment - sedimentCapacity) * depositSpeed;
				sediment -= amountToDeposit;

				// Add the sediment to the four nodes of the current cell using bilinear interpolation
				// Deposition is not distributed over a radius (like erosion) so that it can fill small pits
				heightmap[nodeX][nodeY] 	+=amountToDeposit * (1 - cellOffsetX) * (1 - cellOffsetY);
				heightmap[nodeX+1][nodeY] 	+=amountToDeposit * cellOffsetX * (1 - cellOffsetY);
				heightmap[nodeX][nodeY+1] 	+=amountToDeposit * (1 - cellOffsetX) * cellOffsetY;
				heightmap[nodeX+1][nodeY+1] +=amountToDeposit * cellOffsetX * cellOffsetY;

				/*
				map[dropletIndex] += amountToDeposit * (1 - cellOffsetX) * (1 - cellOffsetY);
				map[dropletIndex + 1] += amountToDeposit * cellOffsetX * (1 - cellOffsetY);
				map[dropletIndex + mapSize] += amountToDeposit * (1 - cellOffsetX) * cellOffsetY;
				map[dropletIndex + mapSize + 1] += amountToDeposit * cellOffsetX * cellOffsetY;
				*/

			} else {
				// Erode a fraction of the droplet's current carry capacity.
				// Clamp the erosion to the change in height so that it doesn't dig a hole in the terrain behind the droplet
				float amountToErode = std::min((sedimentCapacity - sediment) * erodeSpeed, -deltaHeight);

				sediment+=gaussianArraySubtract(heightmap, (float**)gaussian_stamp_K5, nodeX, nodeY, MAP_SIZE, amountToErode);

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
			speed = sqrt(speed * speed + deltaHeight * 4.0f); // gravity
			water *= (1 - evaporateSpeed);
		}
	}
}