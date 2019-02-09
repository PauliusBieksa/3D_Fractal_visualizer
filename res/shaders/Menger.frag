#version 440

uniform float control1;
uniform float aspect_ratio;

layout(location = 1) in vec4 clip_pos;

layout(location = 0) out vec4 out_colour;


// Largest component of a vec3
float maxcomp(in vec3 v)
{
	return max(v.x, max(v.y, v.z));
}

// Get the distance to a cube of 'dims' dimentions at (0, 0) from 'pos'
float sdBox(vec3 pos, vec3 dims)
{
	vec3 di = abs(pos) - dims;
	// If negative get the distance to closest surface
	float mc = maxcomp(di);
	return min(mc, length(max(di, 0.0)));
}

const mat3 y30_rot = mat3(0.6, 0.0, 0.8,
						  0.0, 1.0, 0.0,
						 -0.8, 0.0, 0.6);


//
vec4 map(in vec3 pos)
{
	float d = sdBox(pos, vec3(1.0));
	vec4 res = vec4(d, 1.0, 0.0, 0.0);
	
	float ani = smoothstep(-0.2, 0.2, -cos(0.5 * control1));
	//float ani = 0.0;
	float off = 1.5 * sin(0.01 * control1);
	//float off = 0.0;
	
	float scaling = 1.0;
	//float scaling = 1.0 + 1.5 * sin(control1);
	for (int m = 0; m < 6; m++)
	{
		pos = mix(pos, y30_rot * (pos + off), ani);
		
		vec3 a = mod(pos * scaling, 2.0) - 1.0;
		scaling *= 3.0;
		vec3 r = abs(1.0 - 3.0 * abs(a));
		float da = max(r.x, r.y);
		float db = max(r.y, r.z);
		float dc = max(r.z, r.x);
		float c = (min(da, min(db, dc)) - 1.0) / scaling;
		
		if (c > d)
		{
			d = c;
			res = vec4(d, min(res.y, 0.2 * da * db * dc), (1.0 + float(m)) / 4.0, 0.0);
		}
	}
	
	return res;
}

// Finds intersection distance along the ray direction (rd)
vec4 intersect(in vec3 ro, in vec3 rd)
{
	float t = 0.0;
	vec4 res = vec4(-1.0);
	vec4 h = vec4(1.0);
	for (int i = 0; i < 64; i++)
	{
		if (h.x < 0.002 || t > 10.0)
			break;
		h = map(ro + rd * t);
		res = vec4(t, h.yzw);
		t += h.x;
	}
	// Return -1 if no intersection found
	if (t > 10.0)
		res = vec4(-1.0);
	return res;
}

//
float soft_shadow(in vec3 ro, in vec3 rd, float mint, float k)
{
	float res = 1.0;
	float t = mint;
	float h = 1.0;
	for (int i = 0; i < 32; i++)
	{
		h = map(ro + rd * t).x;
		res = min(res, k * h / t);
		t += clamp(h, 0.005, 0.1);
	}
	return clamp(res, 0.0, 1.0);
}

// Calculates normal by sampling nearby positions
vec3 calc_normal(in vec3 pos)
{
	vec3 eps = vec3(0.001, 0.0, 0.0);
	vec3 nor;
	nor.x = map(pos + eps.xyy).x - map(pos - eps.xyy).x;
	nor.y = map(pos + eps.yxy).x - map(pos - eps.yxy).x;
	nor.z = map(pos + eps.yyx).x - map(pos - eps.yyx).x;
	return normalize(nor);
}

//Light
vec3 light_dir = normalize(vec3(1.0, 0.9, 0.3));

// Determines the colour of the pixel
vec3 render(in vec3 ro, in vec3 rd)
{
	// Background colour
	vec3 col = vec3(1.0, 1.0, 1.0);
	
	// tmat?????
	vec4 tmat = intersect(ro, rd);
	if (tmat.x > 0.0)
	{
		vec3 pos = ro + tmat.x * rd;
		vec3 nor = calc_normal(pos);
		
		float occ = tmat.y;
		float sha = soft_shadow(pos, light_dir, 0.01, 64.0);
		
		float dif = max(0.1 + 0.9 * dot(nor, light_dir), 0.0);
		float sky = 0.5 + 0.5 * nor.y;
		float bac = max(0.4 + 0.6 * dot(nor, vec3(-light_dir.x, light_dir.y, -light_dir.z)), 0.0);
		
		vec3 lin = vec3(0.0);
		lin += 1.0 * dif * vec3(1.1, 0.85, 0.6) * sha;
		lin += 0.5 * sky * vec3(0.1, 0.2, 0.4) * occ;
		lin += 0.1 * bac * vec3(1.0, 1.0, 1.0) * (0.5 + 0.5 * occ);
		lin += 0.25 * occ * vec3(0.15, 0.17, 0.2);
		
		vec3 matcol = vec3(0.5 + 0.5 * cos(0.0 + 2.0 * tmat.z),
						   0.5 + 0.5 * cos(1.0 + 2.0 * tmat.z),
						   0.5 + 0.5 * cos(2.0 + 230 * tmat.z));
		col = matcol * lin;
	}
	
	return pow(col, vec3(0.4545));
}


void main()
{
	// Clip spcase coordinates are interpolated through the layout, instead of calculating them here.
	vec3 pos = clip_pos.xyz;
	pos.x *= aspect_ratio;
	
	
    // camera
    vec3 ro = 1.1*vec3(1.0, 2.0, -4.0); // ray origin
    vec3 ww = normalize(vec3(0.0) - ro); // forward?
    vec3 uu = normalize(cross(vec3(0.0, 1.0, 0.0), ww )); // left?
    vec3 vv = normalize(cross(ww, uu)); // up?
    vec3 rd = normalize( pos.x * uu + pos.y * vv + 2.5 * ww ); // ray direction
	
	
	vec3 colour = render(ro, rd);
	out_colour = vec4(colour, 1.0);
  //  out_colour = vec4(clip_pos.x, clip_pos.y, 0.0, 1.0);
    //out_colour = vec4(clip_pos.x, clip_pos.y, 0.0, 1.0);
}