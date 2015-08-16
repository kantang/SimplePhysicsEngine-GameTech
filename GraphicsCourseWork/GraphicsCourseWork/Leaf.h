#pragma once
#include "SceneNode.h"
class Leaf :
	public SceneNode
{
public:
	//use a quad as a mesh for the leaf in the project, can use more complex primitive for the scene node in future. 
	Leaf(Mesh* m = NULL, Vector4 colour = Vector4(1, 1, 1, 1)) :SceneNode(m, colour){ leafHeight = 0; };
	virtual ~Leaf(void){};

	//set the height of the leave on the branch
	void setLeafHeight(float height){ leafHeight = height; };

	//get the height of the leave on the branch
	float getLeafHeight(){ return leafHeight; };

protected:
	float leafHeight;
};

