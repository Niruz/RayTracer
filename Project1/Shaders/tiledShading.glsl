#version 430 

//Most of these should be uniforms or other
#define MAX_WORK_GROUP_SIZE 16
#define SCREEN_WIDTH 1280.0f
#define SCREEN_HEIGHT 720.0f
#define ASPECT_RATIO = SCREEN_WIDTH/SCREEN_HEIGHT
#define M_PI = 3.14159265358979323846
#define fov = 45.0f * (M_PI/180.0f)
#define NUM_OF_LIGHTS 1
#define NUM_OF_SPHERES 4
#define NUM_OF_TRIANGLES 8
#define MT
//#define FAKE_RECURSION
layout(local_size_x = MAX_WORK_GROUP_SIZE, local_size_y = MAX_WORK_GROUP_SIZE) in;
layout(binding = 0, rgba32f) uniform writeonly image2D finalImage;


struct PointLight
{
	vec4 position;
	vec4 color;
	vec4 attenuationFactors;
	float radius;
};

struct Sphere
{
	vec4 position;
	vec4 color;
	float radius;
	int material;
};

struct Triangle
{
	vec4 v0;
	vec4 v1;
	vec4 v2;
	vec4 tuv;
	vec4 color;
	int material;
};

layout(std430, binding = 5) buffer BufferObject
{
	PointLight pointLights[];
};

layout(std430, binding = 4) buffer BufferObject2
{
	Sphere spheres[];
};

layout(std430, binding = 6) buffer BufferObject3
{
	Triangle triangles[];
};

struct Camera
{
	vec3 position;
	vec3 direction;
};

uniform vec3 cameraPos;
uniform vec3 cameraDirection;
uniform vec3 up;
uniform mat4 viewMatrix;
uniform mat4 inverseViewMatrix;

vec4 backgroundColor = vec4(0.372549f,0.623529f,0.623529f,1.0f);
int maxDepth = 3;
float kEpsilon = 1e-8; 
float bias = 0.0001f;
vec2 getNDCoordinates(ivec2 pixelPos)
{
	return vec2((float(pixelPos.x) + 0.5f)/SCREEN_WIDTH, (float(pixelPos.y) + 0.5f)/SCREEN_HEIGHT);
}

vec2 getPixelScreenCoordinates(ivec2 pixelPos)
{
	vec2 NDC = getNDCoordinates(pixelPos);
	NDC.x = (2.0f * NDC.x - 1.0f);
	//NDC.y = 1.0f - NDC.y * 2.0f;
	NDC.y = (2.0f * NDC.y - 1.0f);
	return NDC;
}

vec2 pixelCameraCoordinates(ivec2 pixelPos)
{
	vec2 camPos = getPixelScreenCoordinates(pixelPos);
	camPos.x = camPos.x/*(SCREEN_WIDTH/SCREEN_HEIGHT) */* tan(radians(45.0f));
	camPos.y = camPos.y* 1.0f/(SCREEN_WIDTH/SCREEN_HEIGHT)* tan(radians(45.0f));
	return camPos;
}

bool intersectsSphere(vec3 origin, vec3 rayDirection, vec3 spherePosition, float sphereRadius, inout float tVal)
{
	vec3 L = spherePosition-origin;

	float tca = dot(L, rayDirection);
	if(tca < 0.0f)
		return false;
	float d = sqrt(dot(L,L) - tca*tca);
	if(d < 0)
	{
		return false;
	}
	if(d > sphereRadius)
	{
		return false;
	}

	float t0, t1;

	float thc = sqrt((sphereRadius*sphereRadius)- d*d);
	t0 = tca - thc;
	t1 = tca + thc;

	if(t0 > t1)
	{
		float temp = t0;
		t0 = t1;
		t1 = temp;
	}
	if(t0 < 0)
	{
		t0 = t1;
		if(t0 <0)
			return false;
			//return vec4(0.0f,0.0f,0.0f,1.0f);
	}

	float t = t0;
	tVal = t;
	return true;
}
vec3 refract(vec3 I, vec3 N, float ior) 
{ 
	float cosi = clamp(dot(I,N), -1,1);
    float etai = 1, etat = ior; 
    vec3 n = N; 
    if (cosi < 0) 
	{ 
		cosi = -cosi;
	}
	else
	{ 
	  float temp = etai;
	  etai = etat;
	  etat = temp;
	  n = -N;
	} 
    float eta = etai / etat; 
    float k = 1 - eta * eta * (1 - cosi * cosi); 
    return k < 0 ? vec3(0) : vec3(eta * I + (eta * cosi - sqrt(k)) * n); 
} 
void fresnel(vec3 I, vec3 N, float ior, inout float kr) 
{ 
	float cosi = clamp(dot(I,N), -1,1);
    float etai = 1, etat = ior; 
    if (cosi > 0) 
	{ 
		float temp = etai;
		etai = etat;
		etat = temp;
	} 

    float sint = etai / etat * sqrt(max(0.f, 1 - cosi * cosi)); 

    if (sint >= 1)
	 { 
        kr = 1; 
    } 
    else 
	{ 
        float cost = sqrt(max(0.f, 1 - sint * sint)); 
        cosi = abs(cosi); 
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost)); 
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost)); 
        kr = (Rs * Rs + Rp * Rp) / 2; 
    } 

} 
bool intersectsPlane(vec3 normal, vec3 planePosition,vec3 rayDirection,inout float t)
{
	vec3 cameraOrigin = cameraPos;
	float denom = dot(normal,rayDirection);

	planePosition = (viewMatrix*vec4(planePosition.xyz,1.0f)).xyz;
	//1e-6
	if(abs(denom) > 0.000001f)
	{
		vec3 p0l0 = planePosition - cameraOrigin;
		t = dot(p0l0, normal) / denom;
		return (t >= 0);
	}

	return false;
}

bool intersectsTriangle(vec3 origin, vec3 direction,
						vec3 v0, vec3 v1, vec3 v2,
						inout float t, inout float u, inout float v,
						inout float T, inout vec3 normal)
{
	

#ifdef MT

	vec3 v0v1 = v1-v0;
	vec3 v0v2 = v2-v0;
	vec3 pvec = cross(direction,v0v2);
	float det = dot(v0v1,pvec);
	if (det <kEpsilon)
		return false;
	if(abs(det) < kEpsilon)
		return false;

	float invDet = 1/det;
	vec3 tvec = origin - v0;

	u = dot(tvec,pvec) *invDet;
	if (u < 0 ||u > 1)
		return false;

	vec3 qvec = cross(tvec,v0v1);
	v = dot(direction,qvec) * invDet;
	if (v < 0 || (u + v) > 1)
		return false;

	t = dot(v0v2,qvec) * invDet;
	T = t;

	normal = normalize(cross(v0v1,v0v2));
	return true;
#else

	vec3 v0v1 = v1 - v0;
	vec3 v0v2 = v2 - v0;

	vec3 N = cross(v0v1,v0v2);
	float denom = dot(N,N);

	float NDotRayDirection = dot(N,direction);
	if(abs(NDotRayDirection) < kEpsilon)
		return false;

	float d = dot(N,v0);

	t = (dot(N,origin) + d) / NDotRayDirection;

	if(t < 0)
		return false;

	vec3 P = origin + t * direction;

	//inside-outside test
	vec3 C;

	//edge 0
	vec3 edge0 = v1 - v0;
	vec3 vp0 = P - v0;
	C = cross(edge0,vp0);
	if(dot(N,C) < 0)
		return false;

	//egde 1
	vec3 edge1 = v2-v1;
	vec3 vp1 = P - v1;
	C = cross(edge1,vp1);
	if(( u = dot(N,C) ) < 0)
		return false;

	//edge 2
	vec3 edge2 = v0 - v2;
	vec3 vp2 = P - v2;
	C = cross(edge2,vp2);
	if((v = dot(N,C)) < 0)
		return false;

	u /= denom;
	v /= denom;

	T = t;
	normal = normalize(N);

	return true;

#endif
}

bool trace(vec3 origin, vec3 rayDirection, inout bool sphereIntersect, inout bool triangleIntersect, inout int index, inout vec3 realTriangleNormal, inout float tNear)
{
	tNear = 99999999.0f;
	index = -1;
	bool sphereIntersection = false;
	bool triangleIntersection = false;
	
	for(int i = 0; i < NUM_OF_SPHERES; i++)
	{
		Sphere sphere = spheres[i];
		sphere.position = viewMatrix*vec4(sphere.position.xyz,1.0f);
		float T = tNear;
		bool intersect = intersectsSphere(origin, rayDirection, sphere.position.xyz, sphere.radius,T);
		if(intersect && (T < tNear))
		{
			tNear = T;
			index = i;
			sphereIntersection = true;
		}
	}
	vec3 triangleNormal;
	for(int i = 0; i < NUM_OF_TRIANGLES; i++)
	{
		
		float t,u,v;
		vec3 v0 = (viewMatrix*triangles[i].v0).xyz;
		vec3 v1 = (viewMatrix*triangles[i].v1).xyz;
		vec3 v2 = (viewMatrix*triangles[i].v2).xyz;
		
		float T = tNear;
		if(intersectsTriangle(origin, rayDirection, v0, v1, v2, t, u, v, T, triangleNormal))
		{	
			if((T < tNear))
			{
				tNear = T;
				index = i;
				sphereIntersection = false;
				triangleIntersection = true;
				realTriangleNormal = triangleNormal;
			}
		}
	}

	sphereIntersect = sphereIntersection;
	triangleIntersect = triangleIntersection;

	return sphereIntersect || triangleIntersect;
}

vec4 calculatePhongShading(vec3 normal, vec3 point, vec4 objectColor, vec3 shadowPointOrig,vec3 origin)
{
	vec3 result = vec3(0.0f,0.0f,0.0f);

	float tNearShadow = 99999999.0f;
	int indexShadow = -1;
	bool sphereIntersectionShadow = false;
	bool triangleIntersectionShadow = false;
	vec3 realTriangleNormalShadow = vec3(0.0f,0.0f,0.0f);

	for(int i = 0; i < NUM_OF_LIGHTS; i++)
	{
		PointLight light = pointLights[i];
		light.position = viewMatrix*light.position;

		vec3 lightPos = light.position.xyz;
		vec3 lightColor = light.color.xyz;
	

		float ambientStrength = 0.1f;
		vec3 ambient = lightColor*ambientStrength;

		vec3 surf2Light = lightPos-point;
		float lightDistance2 = dot(surf2Light,surf2Light);
		vec3 surfaceToLight = normalize(lightPos-point);
	
		float diff = max(dot(normal,surfaceToLight),0.0);
		vec3 diffuse = diff * lightColor;

		float specularStrength = 0.5f;
		vec3 viewDir = normalize(origin - point);
		vec3 reflectDir = reflect(-surfaceToLight, normal);

		float spec = pow(max(dot(viewDir,reflectDir), 0.0), 32);
		vec3 specular = specularStrength * spec * lightColor;

		    // attenuation
		float distance    = length(lightPos - point);
		float attenuation = 1.0 / (light.attenuationFactors.x + light.attenuationFactors.y * distance + 
  				     light.attenuationFactors.y * (distance * distance));   
		
		
		bool shadowCaster = false;
		bool inShadow = trace(shadowPointOrig, surfaceToLight, sphereIntersectionShadow, triangleIntersectionShadow, indexShadow,realTriangleNormalShadow, tNearShadow);
		inShadow = inShadow && (tNearShadow*tNearShadow < lightDistance2);

		int shadowMaterial = -1;
		if(sphereIntersectionShadow)
			shadowMaterial = spheres[indexShadow].material;
		if(triangleIntersectionShadow)
			shadowMaterial = triangles[indexShadow].material;

		vec3 shadow = vec3(1.0f,1.0f,1.0f);
		if(inShadow )
			shadow = vec3(0.1f,0.1f,0.1f);
		if(inShadow && (shadowMaterial == 2))
			shadow = vec3(0.7f,0.7f,0.7f);
		result += ((ambient + diffuse + specular) * attenuation) * objectColor.xyz * shadow;

	}
	
	return vec4(result.xyz,1.0f);
}
vec4 castRay4(vec3 origin, vec3 rayDirection, int depth)
{
	if(depth > maxDepth)
		return backgroundColor;

	vec4 color = vec4(0.0f,0.0f,0.0f,1.0f);
	float tNear = 99999999.0f;
	int index = -1;
	bool sphereIntersection = false;
	bool triangleIntersection = false;
	vec3 realTriangleNormal = vec3(0.0f,0.0f,0.0f);

	bool shadowCaster = false;
	trace(origin, rayDirection, sphereIntersection, triangleIntersection, index,realTriangleNormal, tNear);
	
	vec3 hitPoint;
	vec3 normal;
	vec4 objectColor;
	int material = -1;
	if(sphereIntersection)
	{
		Sphere sphere = spheres[index];
		sphere.position = viewMatrix*vec4(sphere.position.xyz,1.0f);
		hitPoint = origin + rayDirection * tNear;
		normal = normalize(hitPoint - sphere.position.xyz);
		objectColor = sphere.color;
		material = sphere.material;
	}
	if(triangleIntersection)
	{
		hitPoint = origin + rayDirection * tNear;
		Triangle triangle = triangles[index];
		normal = realTriangleNormal;
		objectColor = triangle.color;
		material = triangle.material;
	}
	if(triangleIntersection || sphereIntersection)
	{
		//diffuse
		if(material == 0)
		{
			vec3 shadowPointOrig = (dot(rayDirection, normal) < 0) ? hitPoint + normal * bias :  hitPoint - normal * bias; 
			color += (calculatePhongShading(normal,hitPoint,objectColor,shadowPointOrig, origin));
		}
		//reflective
		else if(material == 1)
		{
			vec3 reflectionVector = reflect(rayDirection, normal);
			vec3 hp = (dot(rayDirection, normal) < 0) ? hitPoint + normal * bias :  hitPoint - normal * bias;
			color += backgroundColor;
		}
		//reflective and refractive
		else if(material == 2)
		{
			color += backgroundColor;
		}
	}
	if(!sphereIntersection && !triangleIntersection)
		color = backgroundColor*0.7;

	return color;
}
vec4 castRay3(vec3 origin, vec3 rayDirection, int depth)
{
	if(depth > maxDepth)
		return backgroundColor;

	vec4 color = vec4(0.0f,0.0f,0.0f,1.0f);
	float tNear = 99999999.0f;
	int index = -1;
	bool sphereIntersection = false;
	bool triangleIntersection = false;
	vec3 realTriangleNormal = vec3(0.0f,0.0f,0.0f);

	bool shadowCaster = false;
	trace(origin, rayDirection, sphereIntersection, triangleIntersection, index,realTriangleNormal, tNear);
	
	vec3 hitPoint;
	vec3 normal;
	vec4 objectColor;
	int material = -1;
	if(sphereIntersection)
	{
		Sphere sphere = spheres[index];
		sphere.position = viewMatrix*vec4(sphere.position.xyz,1.0f);
		hitPoint = origin + rayDirection * tNear;
		normal = normalize(hitPoint - sphere.position.xyz);
		objectColor = sphere.color;
		material = sphere.material;
	}
	if(triangleIntersection)
	{
		hitPoint = origin + rayDirection * tNear;
		Triangle triangle = triangles[index];
		normal = realTriangleNormal;
		objectColor = triangle.color;
		material = triangle.material;
	}
	if(triangleIntersection || sphereIntersection)
	{
		//diffuse
		if(material == 0)
		{
			vec3 shadowPointOrig = (dot(rayDirection, normal) < 0) ? hitPoint + normal * bias :  hitPoint - normal * bias; 
			color += (calculatePhongShading(normal,hitPoint,objectColor,shadowPointOrig, origin));
		}
		//reflective
		else if(material == 1)
		{
			vec3 reflectionVector = reflect(rayDirection, normal);
			vec3 hp = (dot(rayDirection, normal) < 0) ? hitPoint + normal * bias :  hitPoint - normal * bias;
			color += castRay4(hp, reflectionVector, depth+1);
		}
		//reflective and refractive
		else if(material == 2)
		{
			vec4 reflectionColor = vec4(0.0f,0.0f,0.0f,1.0f);
			vec4 refractionColor = vec4(0.0f,0.0f,0.0f,1.0f);
			float kr;
			fresnel(rayDirection,normal,1.5f,kr);

			bool outside = dot(rayDirection,normal) < 0;
			vec3 biasVec = bias * normal;

			if(kr < 1)
			{
				vec3 refractionDirection = normalize(refract(rayDirection, normal, /*isect.hitObject->ior*/1.5f)); 
				vec3 refractionRayOrigin = outside ? hitPoint - biasVec : hitPoint + biasVec;
				refractionColor = castRay4(refractionRayOrigin, refractionDirection, depth +1);

			}

			vec3 reflectionDirection = normalize(reflect(rayDirection, normal));
			vec3 reflectionRayOrigin = outside ? hitPoint + biasVec : hitPoint - biasVec;
			reflectionColor = castRay4(reflectionRayOrigin, reflectionDirection, depth +1);

			color += reflectionColor * kr + refractionColor * (1 -kr);
		}
	}
	if(!sphereIntersection && !triangleIntersection)
		color = backgroundColor*0.7;

	return color;
}
vec4 castRay2(vec3 origin, vec3 rayDirection, int depth)
{
	if(depth > maxDepth)
		return backgroundColor;

	vec4 color = vec4(0.0f,0.0f,0.0f,1.0f);
	float tNear = 99999999.0f;
	int index = -1;
	bool sphereIntersection = false;
	bool triangleIntersection = false;
	vec3 realTriangleNormal = vec3(0.0f,0.0f,0.0f);

	bool shadowCaster = false;
	trace(origin, rayDirection, sphereIntersection, triangleIntersection, index,realTriangleNormal, tNear);
	
	vec3 hitPoint;
	vec3 normal;
	vec4 objectColor;
	int material = -1;
	if(sphereIntersection)
	{
		Sphere sphere = spheres[index];
		sphere.position = viewMatrix*vec4(sphere.position.xyz,1.0f);
		hitPoint = origin + rayDirection * tNear;
		normal = normalize(hitPoint - sphere.position.xyz);
		objectColor = sphere.color;
		material = sphere.material;
	}
	if(triangleIntersection)
	{
		hitPoint = origin + rayDirection * tNear;
		Triangle triangle = triangles[index];
		normal = realTriangleNormal;
		objectColor = triangle.color;
		material = triangle.material;
	}
	if(triangleIntersection || sphereIntersection)
	{
		//diffuse
		if(material == 0)
		{
			vec3 shadowPointOrig = (dot(rayDirection, normal) < 0) ? hitPoint + normal * bias :  hitPoint - normal * bias; 
			color += (calculatePhongShading(normal,hitPoint,objectColor,shadowPointOrig, origin));
		}
		//reflective
		else if(material == 1)
		{
			vec3 reflectionVector = reflect(rayDirection, normal);
			vec3 hp = (dot(rayDirection, normal) < 0) ? hitPoint + normal * bias :  hitPoint - normal * bias;
			color += castRay3(hp, reflectionVector, depth+1);
		}
		//reflective and refractive
		else if(material == 2)
		{
			vec4 reflectionColor = vec4(0.0f,0.0f,0.0f,1.0f);
			vec4 refractionColor = vec4(0.0f,0.0f,0.0f,1.0f);
			float kr;
			fresnel(rayDirection,normal,1.5f,kr);

			bool outside = dot(rayDirection,normal) < 0;
			vec3 biasVec = bias * normal;

			if(kr < 1)
			{
				vec3 refractionDirection = normalize(refract(rayDirection, normal, /*isect.hitObject->ior*/1.5f)); 
				vec3 refractionRayOrigin = outside ? hitPoint - biasVec : hitPoint + biasVec;
				refractionColor = castRay3(refractionRayOrigin, refractionDirection, depth +1);
			}

			vec3 reflectionDirection = normalize(reflect(rayDirection, normal));
			vec3 reflectionRayOrigin = outside ? hitPoint + biasVec : hitPoint - biasVec;
			reflectionColor = castRay3(reflectionRayOrigin, reflectionDirection, depth +1);

			color += reflectionColor * kr + refractionColor * (1 -kr);
		}
	}
	if(!sphereIntersection && !triangleIntersection)
		color = backgroundColor*0.7;

	return color;
}
vec4 castRay(vec3 origin, vec3 rayDirection, int depth)
{
	if(depth > maxDepth)
		return backgroundColor;

	vec4 color = vec4(0.0f,0.0f,0.0f,1.0f);
	float tNear = 99999999.0f;
	int index = -1;
	bool sphereIntersection = false;
	bool triangleIntersection = false;
	vec3 realTriangleNormal = vec3(0.0f,0.0f,0.0f);

	bool shadowCaster = false;
	trace(origin, rayDirection, sphereIntersection, triangleIntersection, index,realTriangleNormal, tNear);
	
	vec3 hitPoint;
	vec3 normal;
	vec4 objectColor;
	int material = -1;
	if(sphereIntersection)
	{
		Sphere sphere = spheres[index];
		sphere.position = viewMatrix*vec4(sphere.position.xyz,1.0f);
		hitPoint = origin + rayDirection * tNear;
		normal = normalize(hitPoint - sphere.position.xyz);
		objectColor = sphere.color;
		material = sphere.material;
	}
	if(triangleIntersection)
	{
		hitPoint = origin + rayDirection * tNear;
		Triangle triangle = triangles[index];
		normal = realTriangleNormal;
		objectColor = triangle.color;
		material = triangle.material;
	}
	if(triangleIntersection || sphereIntersection)
	{
		//diffuse
		if(material == 0)
		{
			vec3 shadowPointOrig = (dot(rayDirection, normal) < 0) ? hitPoint + normal * bias :  hitPoint - normal * bias; 
			color += (calculatePhongShading(normal,hitPoint,objectColor,shadowPointOrig,cameraPos));
		}
		//reflective
		else if(material == 1)
		{
			vec3 reflectionVector = reflect(rayDirection, normal);
			vec3 hp = (dot(rayDirection, normal) < 0) ? hitPoint + normal * bias :  hitPoint - normal * bias;
			color += castRay2(hp, reflectionVector, depth+1);
		}
		//reflective and refractive
		else if(material == 2)
		{
			vec4 reflectionColor = vec4(0.0f,0.0f,0.0f,1.0f);
			vec4 refractionColor = vec4(0.0f,0.0f,0.0f,1.0f);
			float kr;
			fresnel(rayDirection,normal,1.5f,kr);

			bool outside = dot(rayDirection,normal) < 0;
			vec3 biasVec = bias * normal;

			if(kr < 1)
			{
				vec3 refractionDirection = normalize(refract(rayDirection, normal, /*isect.hitObject->ior*/1.5f)); 
				vec3 refractionRayOrigin = outside ? hitPoint - biasVec : hitPoint + biasVec;
				refractionColor = castRay2(refractionRayOrigin, refractionDirection, depth +1);

			}

			vec3 reflectionDirection = normalize(reflect(rayDirection, normal));
			vec3 reflectionRayOrigin = outside ? hitPoint + biasVec : hitPoint - biasVec;
			reflectionColor = castRay2(reflectionRayOrigin, reflectionDirection, depth +1);

			color += reflectionColor * kr + refractionColor * (1 -kr);
		}
	}
	if(!sphereIntersection && !triangleIntersection)
		color = backgroundColor;

	return color;
}

vec4 castRayNonRecursive(inout vec3 origin, inout vec3 rayDirection, inout bool done, int depth)
{
	if(depth == 1)
		return backgroundColor;
	vec4 color = vec4(0.0f,0.0f,0.0f,1.0f);
	float tNear = 99999999.0f;
	int index = -1;
	bool sphereIntersection = false;
	bool triangleIntersection = false;
	vec3 realTriangleNormal = vec3(0.0f,0.0f,0.0f);

	bool shadowCaster = false;
	trace(origin, rayDirection, sphereIntersection, triangleIntersection, index,realTriangleNormal, tNear);
	
	vec3 hitPoint;
	vec3 normal;
	vec4 objectColor;
	int material = -1;

	if(sphereIntersection)
	{
		Sphere sphere = spheres[index];
		sphere.position = viewMatrix*vec4(sphere.position.xyz,1.0f);
		hitPoint = origin + rayDirection * tNear;
		normal = normalize(hitPoint - sphere.position.xyz);
		objectColor = sphere.color;
		material = sphere.material;
	}
	if(triangleIntersection)
	{
		hitPoint = origin + rayDirection * tNear;
		Triangle triangle = triangles[index];
		normal = realTriangleNormal;
		objectColor = triangle.color;
		material = triangle.material;
	}
	if(triangleIntersection || sphereIntersection)
	{
		//diffuse
		if(material == 0)
		{
			vec3 shadowPointOrig = (dot(rayDirection, normal) < 0) ? hitPoint + normal * bias :  hitPoint - normal * bias; 
			color += (calculatePhongShading(normal,hitPoint,objectColor,shadowPointOrig,cameraPos));
			done = true;
		}
		//reflective
		else if(material == 1)
		{
			vec3 reflectionVector = reflect(rayDirection, normal);
			vec3 hp = (dot(rayDirection, normal) < 0) ? hitPoint + normal * bias :  hitPoint - normal * bias;
			origin = hp;
			rayDirection = reflectionVector;
		}
		//reflective and refractive
		else if(material == 2)
		{
			vec4 reflectionColor = vec4(0.0f,0.0f,0.0f,1.0f);
			vec4 refractionColor = vec4(0.0f,0.0f,0.0f,1.0f);
			float kr;
			fresnel(rayDirection,normal,1.5f,kr);

			bool outside = dot(rayDirection,normal) < 0;
			vec3 biasVec = bias * normal;

			//krOut = kr;
			if(kr < 1)
			{
				vec3 refractionDirection = normalize(refract(rayDirection, normal, /*isect.hitObject->ior*/1.5f)); 
				vec3 refractionRayOrigin = outside ? hitPoint - biasVec : hitPoint + biasVec;
				origin = refractionRayOrigin;
				rayDirection = refractionDirection;

			//	refractionColor = castRay2(refractionRayOrigin, refractionDirection, depth +1);
			}
			vec3 reflectionDirection = normalize(reflect(rayDirection, normal));
			vec3 reflectionRayOrigin = outside ? hitPoint + biasVec : hitPoint - biasVec;

		//	reflectDir = reflectionDirection;
		//	reflectOri = reflectionRayOrigin;
		//	origin = reflectionRayOrigin;
		//	rayDirection = reflectionDirection;
			//reflectionColor = castRay2(reflectionRayOrigin, reflectionDirection, depth +1);

		//	color += reflectionColor * kr + refractionColor * (1 -kr);
		//	hitReflectedRay = true;
			//inout vec3 refractDir, inout vec3 refractOri, inout vec3 reflectDir, inout vec3 reflectOri, 
		}
	}
	if(!sphereIntersection && !triangleIntersection)
	{
		color = backgroundColor;
		done = true;
	}


	return color;
}
void main()
{
	ivec2 pixelPosition = ivec2(gl_GlobalInvocationID.xy);
	vec2 camPixelPos = pixelCameraCoordinates(pixelPosition);
	vec3 cameraCoords = vec3(camPixelPos.x, camPixelPos.y, -1.0f);

	vec3 rayOrigin = vec3(0.0f,0.0f,0.0f);
	vec3 rayDirection = cameraCoords - rayOrigin;
	rayDirection = normalize(rayDirection);

	vec4 color = vec4(0.0f,0.0f,0.0f,1.0f);
	#ifdef FAKE_RECURSION
		color = castRay(rayOrigin, rayDirection, 0);
	#else
		bool done = false;

		for(int i = 5; i > 0; i--)
		{
			color = castRayNonRecursive(rayOrigin, rayDirection,done, i);
			if(done)
				break;
		}
	#endif

	float gamma = 2.2;
	vec3 gammaCorrected = pow(color.xyz, vec3(1.0f/gamma));
    color.xyz = gammaCorrected.xyz;

	barrier();
	imageStore(finalImage, pixelPosition, color);
};