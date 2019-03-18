#version 440

uniform float control1;
uniform float control2;
uniform float cam_rotate;
uniform float pitch;

uniform float aspect_ratio;
uniform sampler2D gradient;


layout(location = 1) in vec4 clip_pos;

layout(location = 0) out vec4 out_colour;



// Constants
#define FAR_CLIP 100
#define CAMERA_DISTANCE 20.0
#define RAYMARCH_ITERATIONS 24
#define IFS_ITERATIONS 16
#define VIGNETTE_EXPOSURE 0.2

// IFS parameters
const float scale = 1.25;
vec3 offset = vec3(-2.0, -1.0, -2.0);
mat3 IFS_matrix; // Matrix applied to shape each IFS iteration

// Unscoped parameters
vec4 fractal_colour = vec4(1.0, 0.0, 0.0, 1.0);
vec4 bg_colour = vec4(0.0, 0.0, 0.5, 1.0);
vec3 IFS_rot_axis = vec3(0.5, 1.0, 3.0);


// Gets rotation matrix from axis and angle
mat3 Get_rot_matrix(vec3 axis, float angle)
{
    // Complicated quaternion maths
    vec4 q = vec4(normalize(axis) * sin(angle), cos(angle));
    vec4 qsq = q * q;
	float xy2 = q.x * q.y * 2.0;
	float xz2 = q.x * q.z * 2.0;
	float yz2 = q.y * q.z * 2.0;
	float wx2 = q.w * q.x * 2.0;
	float wy2 = q.w * q.y * 2.0;
	float wz2 = q.w * q.z * 2.0;
	
	return mat3(
		qsq.w +qsq.x - qsq.y -qsq.z,	xy2 - wz2,	xz2 + wy2,
		xy2 + wz2,	qsq.w - qsq.x + qsq.y - qsq.z,	yz2 - wx2,
		xz2 - wy2,	yz2 + wx2,	qsq.w - qsq.x - qsq.y + qsq.z
	);
}

//
vec2 Map(vec3 pos)
{
    float closest = FAR_CLIP;

    float total_scale = 1.0;
    for (int i = 0; i < IFS_ITERATIONS; i++)
    {
        pos = abs(pos);
        pos *= scale;
        total_scale *= scale;
        pos += offset;
        pos = pos * IFS_matrix;

        float cur_dist = length(pos) * total_scale;
        closest = min(closest, cur_dist);
    }
    float l = length(pos) / total_scale;

    // Add fill around individual points in the shape
    float dist = l - 0.07;
    return vec2(dist, closest);
}

//
vec4 Raycast(vec3 ro, vec3 rd)
{
    float closest = FAR_CLIP;
    vec2 d = vec2(0.0);
    float t = 0.01;
    for (int i = 0; i < RAYMARCH_ITERATIONS; i++)
    {
        d = Map(ro + rd * t);
        closest = min(closest, d.x / t);
        if (abs(d.x) < 0.0001)
        {
            break;
        }
        t +=  d.x;
        if (t > FAR_CLIP)
        {
            t = FAR_CLIP;
            break;
        }
    }
    return vec4(t, d.x, d.y, closest);
}

// Returns the normal by sampling 6 positions around the given position
vec3 Get_normal(vec3 pos)
{
    const vec3 eps = vec3(0.00001, 0.0, 0.0);

    return normalize(vec3(
        Map(pos + eps.xyy).x - Map(pos - eps.xyy).x,
        Map(pos + eps.yxy).x - Map(pos - eps.yxy).x,
        Map(pos + eps.yyx).x - Map(pos - eps.yyx).x
        ));
}

// Returns colour based on ray collision
vec4 Trace_ray(vec3 ro, vec3 rd)
{
    vec4 hit = Raycast(ro, rd);
    vec3 hit_pos = ro + rd * hit.x;
    vec3 hit_normal = Get_normal(hit_pos);

    float shade = 1.0;
    float glow = 0.0;
    vec3 env_dir = rd;

    // Ray has hit the fractal and not far plane
    if (hit.x < FAR_CLIP)
    {
        glow = clamp(hit.z / CAMERA_DISTANCE, 0.0, 0.9);
        glow = pow(glow, 5.0);
        shade = glow;
    }

    vec4 colour = vec4(vec3(0.25 + shade * 0.75) * 0.7 , 1.0);
    if (hit.x < FAR_CLIP)
        colour += fractal_colour * 10.0 * glow;

    // Outer glow
    {
        float f = 1.0 - clamp(hit.w * 0.5, 0.0, 1.0);
        float f1 = pow(f, 20.0);
        float glow_amount = f1 * (0.5 + shade * 0.5);;
        colour += fractal_colour * glow_amount;
    }

    return colour;
}

// Applies a vignette effect to the image
vec4 Vignette(vec4 col, vec2 pos)
{
    vec4 result = col;
    result *= clamp(1.0 - dot(pos / aspect_ratio, pos) * 0.4, 0.0, 1.0);
    result = 1.0 - exp(result * -VIGNETTE_EXPOSURE);
    return pow(result, vec4(vec3(1.0 / 2.2), 1.0));
}

void main()
{
	// Clip space coordinate interpolated through the layout from vertex shader
	vec2 pos = clip_pos.xy;
	pos.x *= aspect_ratio;

	// Camera
	vec3 ro = vec3(sin(cam_rotate), sin(cam_rotate * 0.7) * 0.7, cos(cam_rotate)) * CAMERA_DISTANCE; // Ray origin - has to not be at center
	vec3 cam_forward = normalize(vec3(0.0) - ro);
    vec3 cam_left = normalize(cross(vec3(0.0, 1.0, 0.0), cam_forward)); // Presumes up to be pos y (relitively)
    vec3 cam_up = normalize(cross(cam_forward, cam_left));

    vec3 rd = normalize(pos.x * cam_left + pos.y * cam_up + cam_forward);

    fractal_colour = texture(gradient, vec2(0.0, pitch));

    mat3 temp_matrix = Get_rot_matrix(vec3(0.1, 1.0, 0.01), control1 * 0.3);
    IFS_rot_axis = IFS_rot_axis * temp_matrix;

    float IFS_rot_angle = sin(control1 * 0.5);
    IFS_matrix = Get_rot_matrix(IFS_rot_axis, IFS_rot_angle);

    vec4 result = Trace_ray(ro, rd);

    result = Vignette(result, pos);
	result.w = 1.0;
	out_colour = result;
}