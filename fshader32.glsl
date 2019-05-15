#version 150 

in vec4 color;
in vec2 vSR;

in vec4 mVertex;

uniform sampler2D texMap;

out vec4 fColor;

void main() 
{ 
    vec4 tMap = texture(texMap, vSR);
    fColor = color + tMap;

	//float x = mVertex.x;
	//float y = mVertex.y;
	//float z = 0.0;
	//float r = 1.0;

	//// If on the surface of the sphere
	//if ((x*x + y * y) <= r * r) {
	//	// Compute the z coordinate of the point on the sphere
	//	z = sqrt(r*r - x * x - y * y);

	//	// Compute the rotated point
	//	//vec4 vrot = vec4(x, y, z, 1.0) * rot;

	//	// Compute the spherical mapping texture coordinate (3D to 2D texture coordinates)
	//	vec2 longitudeLatitude = vec2(
	//		(atan(vrot.y, vrot.x) / 3.1415926 + 1.0) * 0.5,
	//		(asin(vrot.z) / 3.1415926 + 0.5)
	//	);

	//	// look up the color of the texture image specified by the uniform "mytexture"
	//	// at the position specified by "longitudeLatitude.x" and
	//	// "longitudeLatitude.y" and return it in "gl_FragColor"
	//	fColor = vec4(0.0, 0.0, 0.0, 1.0);
	//}
	//else {
	//	fColor = color + tMap;
	//}
} 
