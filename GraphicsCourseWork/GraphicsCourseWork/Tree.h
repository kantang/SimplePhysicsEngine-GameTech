#pragma once
#include "SceneNode.h"
#include <string>
#include "Trunk.h"
#include "Branch.h"
#include "Leaf.h"
#include <vector>
#include "ParticleEmitter.h"

struct tree_attribute
{
	//the number of the branches that derives from the main branch.
	int numberOfBranches;
	//the length of the tree's main branch in total before bending.
	float length;
	//the average diameter of the tree before branching.
	float diameter;
	//how much a tree's branch fluctuates when growing. from 0.0 to 1.0
	float diversity;
	//how much a tree bends, from 0.0 to 1.0.
	float bendRate;
	//how many bends per branch
	int bendCount;
	//texture number of the branch
	GLuint branchTextureInt;
	//bump map of the branch;
	GLuint branchBumpMap;
	//texture number of the leaves
	GLuint leavesTextureInt;
	//texture number of the flowers
	GLuint flowerTextureInt;
	//texture number of the fruit
	GLuint fruitTextureInt;
};

#define MAX_TRUNK_LENGTH 20.0f
#define MIN_TRUNK_DIAMETER 5.0f
#define TRUNK_LENGTH_WHEN_BEND 5.0f
#define MAX_BEND_TRUNK 2
#define MAX_BRANCH_LENGTH_PROPOTION 2
#define TRUNK_BOTTOM_SHRINK_RATE 1.1f
#define TRUNK_TOP_SHRINK_RATE 1.19f
#define MINIMUM_TREE_SCALE 0.1f
#define MAXIMUM_TREE_SCALE 5.0f
#define TRUNK_TEXTURE TEXTUREDIR"Tree Bark02.JPG"
#define BRANCH_ROTATING_DEGREE 30
#define LEAF_SIZE 4.0f

class Tree :
	public SceneNode
{
public:
	Tree(){};
	//use the tree attribute to initialize the tree
	Tree(tree_attribute attr){ this->createTree(attr); };
	~Tree();
	//set the tree attribute, and create the tree using the attribute
	void setTreeAttribute(tree_attribute attr){ this->attr = attr;  this->createTree(attr); };
	//generate a tree using the attribute
	static Tree* generateTree(tree_attribute attr);

	tree_attribute GetTreeAttribute(){ return attr; };

	//grow the tree by time
	void grow(float msec);
	//reset the growth of the tree
	void resetTree();

	//get the flowers
	vector<SceneNode*>& getFlowers(){ return flowers; };

	//get all the branches
	vector<SceneNode*>  getBranches(){ return allBranches; };

	unsigned int getMemory(){ return memory; };

	Vector3 GetMainTrunkScale(){ return mainTrunks[0]->GetModelScale(); };

protected:
	//calculate the memory usage of the tree
	void calculateMemory();
	//store the total time passed for the tree
	float timePassed;
	//create the tree using the attribute
	void createTree(tree_attribute attr);
	//create the bend
	//this is used for trees that are not straight up
	float createBend(float bottomDiameter, float& currentHeight, SceneNode** previousNode, Matrix4& rotationMatrix,Vector3& translationVec, int numberOfTrunks);
	//create a branch SceneNode
	SceneNode *createBranch(float branchLength, float diameter, float currentHeight, int layer, int numberOfBranches);

	//generate leaves, put them on branchNode and store in the vector
	void generateLeavesOnBranch(SceneNode* branchNode, const Branch* branch);
	//generate flowers, put them on branchNode and store in the vector
	void generateFlowerOnBranch(SceneNode* branchNode, const Branch* branch);

	//the vector that stores all the main trunks
	vector<SceneNode*> mainTrunks;
	//the vector that stores all the branches
	vector<SceneNode*> allBranches;
	//the vector that stores all the main trunks that has branches on them
	vector<SceneNode*> allWithBranches;
	//the vector that stores all the branches with leaves on them
	vector<SceneNode*> allTrunksWithLeaves;
	//the vector that stores all the leaves scene node
	vector<SceneNode*> leaves;
	//the vector that stores all the flowers scene node
	vector<SceneNode*> flowers;
	//the vector that stores all the fruits scene node
	vector<SceneNode*> fruits;
	//the vector that stores all the branches with flowers on them
	vector<SceneNode*> allTrunksWithFlowers;
	//store the attribute of the tree
	tree_attribute attr;
	//store the degree of previous bending angle, use this to
	//avoid multiple bends in the same or similar direction
	short lastBend;
	
	//total memory usage
	unsigned int memory;
};

