#include "stdafx.h"
#include "Tree.h"

Tree::~Tree()
{
}

Tree* Tree::generateTree(tree_attribute attr)
{
	return new Tree(attr);
}

void Tree::createTree(tree_attribute attr)
{
	lastBend = -1;
	mainTrunks = vector<SceneNode*>(0);
	allBranches = vector<SceneNode*>(0);
	allWithBranches = vector<SceneNode*>(0);
	allTrunksWithLeaves = vector<SceneNode*>(0);
	allTrunksWithFlowers = vector<SceneNode*>(0);
	leaves = vector<SceneNode*>(0);
	flowers = vector<SceneNode*>(0);
	fruits = vector<SceneNode*>(0);
	boundingRadius = attr.length;
	int numberOfBranches = attr.numberOfBranches;
	int trunkCount = 0;
	//create a root trunk
	SceneNode *rootTrunk = new SceneNode(new Trunk(attr.branchTextureInt, attr.branchBumpMap, Vector3(0.0f, MAX_TRUNK_LENGTH, 0.0f), Vector3(0, 1.0f, 0), attr.diameter, attr.diameter * 1.1f, MAX_TRUNK_LENGTH, 0), Vector4(0, 0, 0, 0));
	rootTrunk->SetBoundingRadius(MAX_TRUNK_LENGTH);
	rootTrunk->SetModelScale(Vector3(0, 0, 0));
	this->AddChild(rootTrunk);
	mainTrunks.push_back(rootTrunk);
	//create a main trunk
	//create the body of the main trunk
	float currentHeight = 0.0f;
	currentHeight += (((Trunk*)rootTrunk->GetMesh())->getMaxHeight());
	float previousTopDiameter = attr.diameter;
	//bend rate lower than 0.05 discarded. if don't bend, don't go through bend code;
	int numberOfTrunksWhenBend = 1 + (int)(MAX_BEND_TRUNK * (attr.bendRate / (TRUNK_LENGTH_WHEN_BEND / MAX_TRUNK_LENGTH)));
	int bendCount = numberOfTrunksWhenBend == 1 ? 0 : attr.bendCount;
	//inside the while loop is the main body of the main trunk, except the top part, draw top part after this is done;
	//pick randomly from the trunk which is above half the height of the tree to generate branches,
	//the branches can be either facing up or down or left or right, each has different growing behaviour.
	SceneNode* previousNode = rootTrunk;
	Vector3 previousTranslation = Vector3(0, MAX_TRUNK_LENGTH, 0);
	Matrix4 previousRotation = Matrix4::Rotation(0, Vector3(0, 0, 0));
	++trunkCount;
	while (currentHeight < attr.length - MAX_TRUNK_LENGTH)
	{
		//calculate the diameter of the top circle
		float shrinkRate = currentHeight > attr.length / 2 ? ((float)TRUNK_TOP_SHRINK_RATE *(rand() % 100 > 80 ? 1.1f : 1.0f)) : ((float)TRUNK_BOTTOM_SHRINK_RATE *(rand() % 100 > 80 ? 1.1f : 1.0f));
		float topDiameter = previousTopDiameter / shrinkRate;
		topDiameter = topDiameter > MIN_TRUNK_DIAMETER ? topDiameter : MIN_TRUNK_DIAMETER;
		//the diameter of the bottom circle is the diameter of the last scene node's top circle
		float bottomDiameter = previousTopDiameter;
		previousTopDiameter = topDiameter;
		//calculate the center of the top.
		//if not yet bend, and the current height is more than half the total length,
		//it has 80% percent chance to bend now.
		//using this percentage only because to grow the diversity of the tree
		if ((bendCount > 1) && (currentHeight > attr.length / 8) && (rand() % 100 > 60))
		{
			if (rand() % 100 > 0 && trunkCount > 1 && numberOfBranches)
			{
				allWithBranches.push_back(previousNode);
				numberOfBranches--;
				trunkCount = 0;
			}
			//the bend is smooth, so each part bend in the same angle until it's done
			previousTopDiameter = previousTopDiameter * shrinkRate;
			bottomDiameter = previousTopDiameter;
			previousTopDiameter = createBend(bottomDiameter, currentHeight, &previousNode, previousRotation,previousTranslation,numberOfTrunksWhenBend);
			--bendCount;
			++trunkCount;
		}
		//if above requirement is not meet, just create a cylinder
		//top normal is equal to bottom normal
		//top center = bottom center + height.
		else
		{
			SceneNode *trunk = new SceneNode(new Trunk(attr.branchTextureInt, attr.branchBumpMap, Vector3(0, MAX_TRUNK_LENGTH, 0), Vector3(0, 1, 0), topDiameter, bottomDiameter, MAX_TRUNK_LENGTH, currentHeight), Vector4(0, 0, 0, 0));
			trunk->SetBoundingRadius(MAX_TRUNK_LENGTH);
			trunk->SetModelScale(Vector3(0, 0, 0));
			trunk->SetTranslation(previousTranslation);
			trunk->SetRotation(previousRotation);
			previousNode->AddChild(trunk);
			previousNode = trunk;
			mainTrunks.push_back(trunk);
			currentHeight += (((Trunk*)trunk->GetMesh())->getMaxHeight());
			previousTranslation = Vector3(0, (((Trunk*)trunk->GetMesh())->getMaxHeight()), 0);
			previousRotation = Matrix4::Rotation(0,Vector3(0,0,0));
			if (trunkCount > 1 && numberOfBranches)
			{
				allWithBranches.push_back(previousNode);
				numberOfBranches--;
				trunkCount = 0;
			}
			++trunkCount;
		}
	}
	//after creating the main body, create the top part of the tree
	float topLength = previousTopDiameter * TRUNK_LENGTH_WHEN_BEND;
	SceneNode *trunk = new SceneNode(new Branch(attr.branchTextureInt, attr.branchBumpMap, previousTopDiameter, attr.length - currentHeight, currentHeight), Vector4(0, 0, 0, 0));
	trunk->SetBoundingRadius(topLength);
	trunk->SetModelScale(Vector3(0, 0, 0));
	trunk->SetTranslation(previousTranslation);
	trunk->SetRotation(previousRotation);
	previousNode->AddChild(trunk);
	mainTrunks.push_back(trunk);
	//create the branches
	for (unsigned int i = 0; i < allWithBranches.size(); i++)
	{
		SceneNode *branchRoot = allWithBranches[i];
		Trunk *rootTrunk = (Trunk*)branchRoot->GetMesh();
		float branchLength = attr.length / 3;
		float diameter = rootTrunk->getTopDiameter();
		float branchDiameter = diameter / 3;
		branchDiameter = branchDiameter > 5.0f ? branchDiameter : 5.0f;
		SceneNode* newBranch = createBranch(branchLength, branchDiameter, rootTrunk->getHeight(), (int)sqrt(attr.numberOfBranches), attr.numberOfBranches / 2);
		//after the new branch is created, add it to current branch.
		//the direction of the branch is commutative for each branch created
		//the direction is rotated 30 degree to -x, x, -z or z axis.
		int direction = (rand()) % 4;
		switch ((++direction) % 4)
		{
		case 1:
			newBranch->SetBoundingRadius(branchLength);
			newBranch->SetModelScale(Vector3(0, 0, 0));
			newBranch->SetTranslation(Vector3((branchDiameter - diameter) / 2, 0, 0));
			newBranch->SetRotation(Matrix4::Rotation((float)(30 + rand() % 15), Vector3(0, 0, 1)));
			branchRoot->AddChild(newBranch);
			break;
		case 2:
			newBranch->SetBoundingRadius(branchLength);
			newBranch->SetModelScale(Vector3(0, 0, 0));
			newBranch->SetTranslation(Vector3((diameter - branchDiameter) / 2, 0, 0));
			newBranch->SetRotation(Matrix4::Rotation((float)(-30 - rand() % 15), Vector3(0, 0, 1)));
			branchRoot->AddChild(newBranch);
			break;
		case 3:
			newBranch->SetBoundingRadius(branchLength);
			newBranch->SetModelScale(Vector3(0, 0, 0));
			newBranch->SetTranslation(Vector3(0, 0, (diameter - branchDiameter) / 2));
			newBranch->SetRotation(Matrix4::Rotation((float)(30 + rand() % 15), Vector3(1, 0, 0)));
			branchRoot->AddChild(newBranch);
			break;
		case 0:
			newBranch->SetBoundingRadius(branchLength);
			newBranch->SetModelScale(Vector3(0, 0, 0));
			newBranch->SetTranslation(Vector3(0, 0, (branchDiameter - diameter) / 2));
			newBranch->SetRotation(Matrix4::Rotation((float)(-30 - rand() % 15), Vector3(1, 0, 0)));
			branchRoot->AddChild(newBranch);
			break;
		default:
			break;
		}
		
	}
	//create the leaves' node
	for (unsigned int k = 0; k < allTrunksWithLeaves.size(); k++)
	{
		SceneNode* currentNode = allTrunksWithLeaves[k];
		Branch* currentBranch = (Branch*)currentNode->GetMesh();
		generateLeavesOnBranch(currentNode, currentBranch);
	}
	//create the flowers' node
	for (unsigned int k = 0; k < allTrunksWithFlowers.size(); k++)
	{
		SceneNode* currentNode = allTrunksWithLeaves[k];
		Branch* currentBranch = (Branch*)currentNode->GetMesh();
		generateFlowerOnBranch(currentNode, currentBranch);
	}
	timePassed = 0;
	calculateMemory();
}

float Tree::createBend(float bottomDiameter, float& currentHeight, SceneNode** previousNode, Matrix4& rotationMatrix,Vector3& translationVec, int numberOfTrunks)
{
	float bendAngle = attr.bendRate * PI / 2 / numberOfTrunks;
	//the first bend has another randomized direction,
	//this is because you can't always bend in one direction.
	//All the following trunk don't have to bend as long as it's the child of former trunk.
	
	short bendDirection = rand() % 360;
	if (lastBend < 0)
		lastBend = bendDirection;
	else
	{
		while (bendDirection > lastBend - 45 && bendDirection < lastBend + 45)
		{
			bendDirection = rand() % 360;
		}
		lastBend = bendDirection;
	}
	float shrinkRate = ((TRUNK_TOP_SHRINK_RATE - 1) / numberOfTrunks) + 1;
	float previousTopDiameter = bottomDiameter;
	float topDiameter = bottomDiameter / shrinkRate;
	int degree = (rand() % 100 > 50) ? 0 : 180;
	while (numberOfTrunks > 0)
	{
		//get the center of the top, it's on the circle which sits in the top surface
		//the coordinate of the circle can be get via Trunk's circle function
		Vector3 topCoord = Trunk::vertexCoordOfCircle(Vector3(0, 1, 0), Vector3(0, TRUNK_LENGTH_WHEN_BEND, 0), tan(bendAngle) *TRUNK_LENGTH_WHEN_BEND, degree);
		//the top normal is equal to the normalized top coordinate.
		Vector3 topNormal = topCoord;
		topNormal.Normalise();
		SceneNode *bendTrunk = new SceneNode(new Trunk(attr.branchTextureInt, attr.branchBumpMap, topCoord, topNormal, topDiameter, bottomDiameter, TRUNK_LENGTH_WHEN_BEND, currentHeight), Vector4(0, 0, 0, 0));
		bendTrunk->SetBoundingRadius(TRUNK_LENGTH_WHEN_BEND);
		bendTrunk->SetModelScale(Vector3(0, 0, 0));
		bendTrunk->SetRotation(rotationMatrix);
		bendTrunk->SetTranslation(translationVec);
		(*previousNode)->AddChild(bendTrunk);
		(*previousNode) = bendTrunk;
		mainTrunks.push_back(bendTrunk);
		//set the bend direction to 0 after the first one.
		bendDirection = 0;
		//modify the top diameter
		previousTopDiameter = topDiameter;
		topDiameter = topDiameter / (rand() % 100 > 90 ? shrinkRate : 1.0f);
		topDiameter = topDiameter > MIN_TRUNK_DIAMETER ? topDiameter : MIN_TRUNK_DIAMETER;
		//modify the bottom diameter
		bottomDiameter = previousTopDiameter;
		--numberOfTrunks;
		currentHeight += (((Trunk*)bendTrunk->GetMesh())->getMaxHeight());
		Vector3 v1 = Vector3(0, -1, 0);
		Vector3 axis = Vector3::Cross(v1, topNormal);
		axis.Normalise();
		rotationMatrix = Matrix4::Rotation((float)RadToDeg(acos(Vector3::Dot(-v1, topNormal))), axis);
		translationVec = topCoord;
	}
	return previousTopDiameter;
}

SceneNode* Tree::createBranch(float length, float diameter, float currentHeight, int layer, int numberOfBranches)
{
	SceneNode* currentBranch = new SceneNode(new Branch(attr.branchTextureInt, attr.branchBumpMap, diameter, length, currentHeight), Vector4(0, 0, 0, 0));
	currentBranch->SetBoundingRadius(length);
	currentBranch->SetModelScale(Vector3(0, 0, 0));
	if (layer != 0)
	{
		//create the branch diameter and the branch length for the next layer.
		float branchDiameter = diameter / 2;
		branchDiameter = branchDiameter > 1 ? branchDiameter : 1.0f;
		float branchLength = length * 1 / 2;
		branchLength = branchLength > 10.0f ? branchLength * 2 / 3 : 10.0f;
		//create the next layer, using this method recursively
		int counter = 0;
		int startAxis = rand() % 4;
		for (int currentBranchHeight = ((int)length / (numberOfBranches)); currentBranchHeight < length; currentBranchHeight += (int)length / (numberOfBranches))
		{
			float heightRate = 1.1f - ((float)currentBranchHeight / (length - 5.0f));
			float currentDiameter = diameter * (heightRate);
			currentDiameter = currentDiameter > 0.2f ? currentDiameter : 0.2f;
			branchDiameter = branchDiameter * (heightRate);
			branchDiameter = branchDiameter > 0.2f ? branchDiameter : 0.2f;
			SceneNode* newBranch = createBranch(branchLength, branchDiameter, currentHeight + currentBranchHeight, layer - 1, numberOfBranches / 2);
			//after the new branch is created, add it to current branch.
			//the direction of the branch is commutative for each branch created
			//the direction is rotated 30 degree to -x, x, -z or z axis.
			switch ((counter + startAxis) % 4)
			{
			case 1:
				newBranch->SetBoundingRadius(branchLength);
				newBranch->SetModelScale(Vector3(0, 0, 0));
				newBranch->SetTranslation(Vector3(0, (float)currentBranchHeight, 0));
				newBranch->SetRotation(Matrix4::Rotation(30, Vector3(0, 0, 1)));
				currentBranch->AddChild(newBranch);
				break;
			case 2:
				newBranch->SetBoundingRadius(branchLength);
				newBranch->SetModelScale(Vector3(0, 0, 0));
				newBranch->SetTranslation(Vector3(0, (float)currentBranchHeight, 0));
				newBranch->SetRotation(Matrix4::Rotation(-30, Vector3(0, 0, 1)));
				currentBranch->AddChild(newBranch);
				break;
			case 3:
				newBranch->SetBoundingRadius(branchLength);
				newBranch->SetModelScale(Vector3(0, 0, 0));
				newBranch->SetTranslation(Vector3(0, (float)currentBranchHeight, 0));
				newBranch->SetRotation(Matrix4::Rotation(30, Vector3(1, 0, 0)));
				currentBranch->AddChild(newBranch);
				break;
			case 0:
				newBranch->SetBoundingRadius(branchLength);
				newBranch->SetModelScale(Vector3(0, 0, 0));
				newBranch->SetTranslation(Vector3(0, (float)currentBranchHeight, 0));
				newBranch->SetRotation(Matrix4::Rotation(-30, Vector3(1, 0, 0)));
				currentBranch->AddChild(newBranch);
				break;
			default:
				break;
			}
			++counter;
		}
		if (layer < 3)
			allTrunksWithLeaves.push_back(currentBranch);
		if (layer == 1)
			allTrunksWithFlowers.push_back(currentBranch);

	}
	else
	{
		allTrunksWithLeaves.push_back(currentBranch);
		allTrunksWithFlowers.push_back(currentBranch);
	}
	allBranches.push_back(currentBranch);
	return currentBranch;
}

void Tree::generateLeavesOnBranch(SceneNode* branchNode, const Branch* branch)
{
	int length = (int)branch->getMaxHeight();
	int leafSets = (int)(length / LEAF_SIZE) ;
	if (leafSets == 0)
		return;
	float height = (float)(length / leafSets / 2);
	for (int k = 1; k < leafSets + 1; k++)
	{
		int fluctuate = (rand() % 2) - 1;
		Leaf *leftLeaveNode = new Leaf(Mesh::GenerateQuad(), Vector4(0, 0, 0, 0));
		leftLeaveNode->GetMesh()->SetTexture(attr.leavesTextureInt);
		leftLeaveNode->SetModelScale(Vector3(0,0,0));
		leftLeaveNode->SetTranslation(Vector3(LEAF_SIZE / 2, height * k + fluctuate, 0));
		leftLeaveNode->SetRotation(Matrix4::Rotation(180.0f, Vector3(0, 0, 1)));
		leftLeaveNode->setLeafHeight(leftLeaveNode->getTranslation().y);
		branchNode->AddChild(leftLeaveNode);
		fluctuate = (rand() % 2) - 1;
		Leaf *rightLeaveNode = new Leaf(Mesh::GenerateQuad(), Vector4(0, 0, 0, 0));
		rightLeaveNode->GetMesh()->SetTexture(attr.leavesTextureInt);
		rightLeaveNode->SetModelScale(Vector3(0, 0, 0));
		rightLeaveNode->SetTranslation(Vector3(-LEAF_SIZE / 2, height * k + fluctuate, 0));
		rightLeaveNode->SetRotation(Matrix4::Rotation(0, Vector3(0, 0, 0)));
		rightLeaveNode->setLeafHeight(rightLeaveNode->getTranslation().y);
		branchNode->AddChild(rightLeaveNode);
		leaves.push_back(leftLeaveNode);
		leaves.push_back(rightLeaveNode);
	}
}

void Tree::generateFlowerOnBranch(SceneNode* branchNode, const Branch* branch)
{
	int length = (int)(branch->getMaxHeight());
	int fluctuate =(int)((rand() % (int)LEAF_SIZE) - LEAF_SIZE + 1);
	SceneNode *flowerNode = new SceneNode(Mesh::GenerateQuad(), Vector4(0, 0, 0, 0));
	flowerNode->GetMesh()->SetTexture(attr.flowerTextureInt);
	flowerNode->SetModelScale(Vector3(0,0,0));
	flowerNode->SetTranslation(Vector3(-LEAF_SIZE / 2, 0.0f + fluctuate, 0));
	flowerNode->SetRotation(Matrix4::Rotation(0, Vector3(0, 0, 1)));
	branchNode->AddChild(flowerNode);
	flowers.push_back(flowerNode);

}

void Tree::grow(float msec)
{
	timePassed += msec;
	float time = timePassed / 1000;
	if (time < 0)
		return;
	if (time < 30)
	{
		float rate = 1.0f / 2.0f;
		float xzGrow = -(pow(rate, time) - 1) * MAXIMUM_TREE_SCALE;
		rate = 1.0f / 1.2f;
		float yGrow = -(pow(rate, time) - 1.01f) * MAXIMUM_TREE_SCALE;
		if (time < 5)
		{
			for (unsigned int k = 0; k < mainTrunks.size(); k++)
			{
				mainTrunks[k]->SetModelScale(Vector3(xzGrow, yGrow, xzGrow));
			}
			
		}
		else
		{
			float btime = time - 5;
			btime *= 20;
			float brate = 1.0f / 3.0f;
			float bxzGrow = -(pow(brate, btime) - 1) * MAXIMUM_TREE_SCALE;
			brate = 1.0f / 1.01f;
			float byGrow = -(pow(brate, btime) - 1) * MAXIMUM_TREE_SCALE;
			for (unsigned int k = 0; k < mainTrunks.size(); k++)
			{
				mainTrunks[k]->SetModelScale(Vector3(xzGrow, yGrow, xzGrow));
			}
			for (unsigned int k = 0; k < allBranches.size(); k++)
			{
				allBranches[k]->SetModelScale(Vector3(bxzGrow, byGrow, bxzGrow));
			}
			if (time > 5)
			{
				for (unsigned int k = 0; k < leaves.size(); k++)
				{
					Leaf *leafNode = (Leaf*)leaves[k];
					Branch* branchNode = (Branch*)leafNode->getParent()->GetMesh();
					if (leafNode->getParent()->GetModelScale().y * branchNode->getMaxHeight() > leafNode->getLeafHeight() * 5 * MAXIMUM_TREE_SCALE && leafNode->GetModelScale().x < 2 * MAXIMUM_TREE_SCALE)
					{
						Vector3 modelScale = leafNode->GetModelScale();
						leafNode->SetModelScale(Vector3(modelScale.x + (msec / 1000)* MAXIMUM_TREE_SCALE, 2 * MAXIMUM_TREE_SCALE, modelScale.z + (msec / 1000)* MAXIMUM_TREE_SCALE));
					}
				}
				if (time > 20)
				{
					float ftime = time - 20;
					float frate = 1.0f / 2.0f;
					float fxzGrow = -(pow(frate, ftime) - 1) * MAXIMUM_TREE_SCALE;
					for (unsigned int k = 0; k < flowers.size(); k++)
						flowers[k]->SetModelScale(Vector3(fxzGrow * 4, 4 * MAXIMUM_TREE_SCALE, fxzGrow * 4));
				}
			}
		}
	}
	else if (time < 60)
	{
		//generate fruit and scale the leaves and flowers down
		if (time < 45)
		{
			for (unsigned int k = 0; k < flowers.size(); k++)
			{
				flowers[k]->SetModelScale(Vector3(MAXIMUM_TREE_SCALE * 4 * (45 - time) / 15, MAXIMUM_TREE_SCALE * 4 * (45 - time) / 15, MAXIMUM_TREE_SCALE * 4 * (45 - time) / 15));
			}
		}
		else
		{
			for (unsigned int k = 0; k < leaves.size(); k++)
			{
				Leaf *leafNode = (Leaf*)leaves[k];
				Branch* branchNode = (Branch*)leafNode->getParent()->GetMesh();
				if (leafNode->getParent()->GetModelScale().y * branchNode->getMaxHeight() > leafNode->getLeafHeight() * 5 * MAXIMUM_TREE_SCALE)
				{
					leafNode->SetModelScale(Vector3(MAXIMUM_TREE_SCALE * 2 * (60 - time) / 15, MAXIMUM_TREE_SCALE * 2 * (60 - time) / 15, MAXIMUM_TREE_SCALE * 2 * (60 - time) / 15));
				}
			}
		}
	}
	else if (time < 105 && time > 75)
	{
		//scale the leaves and flowers up again
		if (time < 90)
		{
			for (unsigned int k = 0; k < leaves.size(); k++)
			{
				Leaf *leafNode = (Leaf*)leaves[k];
				Branch* branchNode = (Branch*)leafNode->getParent()->GetMesh();
				if (leafNode->getParent()->GetModelScale().y * branchNode->getMaxHeight() > leafNode->getLeafHeight() * 5 * MAXIMUM_TREE_SCALE)
				{
					leafNode->SetModelScale(Vector3(MAXIMUM_TREE_SCALE * 2 * (time - 75) / 15, MAXIMUM_TREE_SCALE * 2 * (time - 75) / 14, MAXIMUM_TREE_SCALE * 2 * (time - 75) / 15));
				}
			}
			
		}
		else
		{
			for (unsigned int k = 0; k < flowers.size(); k++)
				flowers[k]->SetModelScale(Vector3(MAXIMUM_TREE_SCALE * 4 * (time - 90) / 15, MAXIMUM_TREE_SCALE * 4 * (time - 90) / 15, MAXIMUM_TREE_SCALE * 4 * (time - 90) / 15));
		}
	}
	else if (time > 75)
	{
		timePassed = 30100;
	}
}

void Tree::resetTree()
{
	timePassed = 0;
	for (unsigned int k = 0; k < mainTrunks.size(); k++)
	{
		mainTrunks[k]->SetModelScale(Vector3(0, 0, 0));
	}
	for (unsigned int k = 0; k < allBranches.size(); k++)
	{
		allBranches[k]->SetModelScale(Vector3(0, 0, 0));
	}
	for (unsigned int k = 0; k < leaves.size(); k++)
	{
		leaves[k]->SetModelScale(Vector3(0, 0, 0));
	}
	for (unsigned int k = 0; k < flowers.size(); k++)
	{
		flowers[k]->SetModelScale(Vector3(0, 0, 0));
	}
	for (unsigned int k = 0; k < fruits.size(); k++)
	{
		fruits[k]->SetModelScale(Vector3(0, 0, 0));
	}
}

void Tree::calculateMemory()
{
	memory = 0;
	if (mainTrunks.size() > 0)
		memory += (mainTrunks[0]->GetMesh()->getMemory() * mainTrunks.size());
	if (allBranches.size() > 0)
		memory += (allBranches[0]->GetMesh()->getMemory() * allBranches.size());
	if (flowers.size() > 0)
		memory += (flowers[0]->GetMesh()->getMemory() * flowers.size());
	if (leaves.size() > 0)
		memory += (leaves[0]->GetMesh()->getMemory() * leaves.size());
	if (fruits.size() > 0)
		memory += (fruits[0]->GetMesh()->getMemory() * fruits.size());

}