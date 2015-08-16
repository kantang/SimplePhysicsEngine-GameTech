#version 150 core



in  vec3 position;
in  vec4 colour;

out Vertex {
	vec4 colour;
} OUT;

vec4 myColour = vec4(1/position.x,1/position.y,1/position.z,1);

void main(void)	{
	//position.x = position.x + 50;
	gl_Position	  = vec4(position, 2);
	OUT.colour    = colour;
	
}
