#version 440

uniform float control1;
uniform float aspect_ratio;

layout(location = 1) in vec4 clip_pos;

layout(location = 0) out vec4 out_colour;



#define k_raymarch_iterations 24
#define k_IFS_iterations 16

float k_exposure = 0.1;

// KIFS parameters
const float f_scale = 1.25;
vec3 v_offset = vec3(-1.0, -2.0, -0.2);
mat3 m;

const float k_far_clip = 30;


const vec3 v_glow_colour = vec3(1.0, 0.075, 0.01) * 5.0;


vec2 Get_scene_distance(in vec3 v_pos)
{
	float f_trap = k_far_clip;
	
	float f_total_scale = 1.0;
	for (int i = 0; i < k_IFS_iterations; i++)
	{
		v_pos = abs(v_pos);
		v_pos *= f_scale;
		f_total_scale *= f_scale;
		v_pos += v_offset;
		v_pos = v_pos * m;
		
		float f_cur_dist = length(v_pos) * f_total_scale;
		f_trap = min(f_trap, f_cur_dist);
	}
	float l = length(v_pos) / f_total_scale;
	
	float f_dist = l - 0.1;
	return vec2(f_dist, f_trap);
}

vec4 Raycast(const in vec3 v_ro, const in vec3 v_rd)
{
	float f_closest = k_far_clip;
	vec2 d = vec2(0.0);
	float t = 0.01;
	for (int i = 0; i < k_raymarch_iterations; i++)
	{
		d = Get_scene_distance(v_ro + v_rd * t);
		f_closest = min(f_closest, d.x / t);
		if (abs(d.x) < 0.0001)
		{
			break;
		}
		t += d.x;
		if (t > k_far_clip)
		{
			t = k_far_clip;
			break;
		}
	}
	return vec4(t, d.x, d.y, f_closest);
}

vec3 Get_scene_normal( const in vec3 v_pos )
{
    const float f_delta = 0.000001;

    vec3 v_offset1 = vec3( f_delta, -f_delta, -f_delta);
    vec3 v_offset2 = vec3(-f_delta, -f_delta,  f_delta);
    vec3 v_offset3 = vec3(-f_delta,  f_delta, -f_delta);
    vec3 v_offset4 = vec3( f_delta,  f_delta,  f_delta);

    float f1 = Get_scene_distance( v_pos + v_offset1 ).x;
    float f2 = Get_scene_distance( v_pos + v_offset2 ).x;
    float f3 = Get_scene_distance( v_pos + v_offset3 ).x;
    float f4 = Get_scene_distance( v_pos + v_offset4 ).x;

    vec3 v_normal = v_offset1 * f1 + v_offset2 * f2 + v_offset3 * f3 + v_offset4 * f4;

    return normalize( v_normal );
}

vec3 Trace_ray(const in vec3 v_ro, const in vec3 v_rd)
{
	vec4 v_hit = Raycast(v_ro, v_rd);
	
	vec3 v_hit_pos = v_ro + v_rd * v_hit.x;
	vec3 v_hit_normal = Get_scene_normal(v_hit_pos);
	
	float f_shade = 1.0;
	float f_glow = 0.0;
	vec3 v_env_dir = v_rd;
	if (v_hit.x < k_far_clip)
	{
		v_env_dir = reflect(v_rd, v_hit_normal);
		f_glow = clamp(v_hit.z * 0.1, 0.0, 1.0);
		f_glow = pow(f_glow, 3.0);
		f_shade = f_glow;
	}
	
	//vec3 v_env = tex
	vec3 v_colour = vec3(0.25 + f_shade * 0.75);
	if (v_hit.x < k_far_clip)
	{
		v_colour += v_glow_colour * 10.0 * f_glow;
	}
	
	// Outer glow
	{
		float f = 1.0 - clamp(v_hit.w * 0.5, 0.0, 1.0);
		
		float f_glow_amount = 0.0;
		
		// Big glow
		float f1 = pow(f, 20.0);
		f_glow_amount += f1 * 2.0 * (0.5 + f_shade * 0.5);
		
		// Small glow
		float f2 = pow(f, 200.0);
		f_glow_amount += f2 * 5.0 * f_shade;
		
		v_colour += v_glow_colour * f_glow_amount;
	}
	
	return v_colour;
}

mat3 Set_rot(const in vec4 q)
{
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

mat3 Set_rot(vec3 v_axis, float f_angle)
{
	return Set_rot(vec4(normalize(v_axis) * f_angle, cos(f_angle)));
}

vec3 Apply_post_fx(const in vec3 v_in, const in vec2 v_uv)
{
	vec3 v_result = v_in;
	v_result *= clamp(1.0 - dot(v_uv / aspect_ratio, v_uv) * 0.4, 0.0, 1.0);
	v_result = 1.0 - exp(v_result * -k_exposure);
	v_result = pow(v_result, vec3(1.0 / 2.2));
	
	return v_result;
}


void main()
{
	// Clip spcase coordinates are interpolated through the layout, instead of calculating them here.
	vec2 pos = clip_pos.xy;
	pos.x *= aspect_ratio;
	
	
	float f_heading = control1 * 0.21;
	float f_elevation = cos(control1 * 0.1) * 0.5;
	float f_camera_distance = 15.0 + sin(control1 * 0.05) * 5.0;
	//float f_heading = 0.5;
	//float f_elevation = 0.5;
	//float f_camera_distance = 15.0;
	
	float f_sin_elevation = sin(f_elevation);
	float f_cos_elevation = cos(f_elevation);
	float f_sin_heading = sin(f_heading);
	float f_cos_heading = cos(f_heading);
	
	vec3 v_camera_offset;
	v_camera_offset.x = f_sin_heading * f_cos_elevation;
	v_camera_offset.y = f_sin_elevation;
	v_camera_offset.z = f_cos_heading * f_cos_elevation;
	
	vec3 v_camera_pos = v_camera_offset * f_camera_distance;
	
	vec3 v_camera_target = vec3(0.0);
	
	vec3 v_forward = normalize(v_camera_target - v_camera_pos);
	vec3 v_right = normalize(cross(vec3(0.0, 1.0, 0.0), v_forward)); // Left??
	vec3 v_up = normalize(cross(v_forward, v_right));
	
	float f_fov = 2.0;
	
	vec3 v_rd = normalize(pos.x * v_right + pos.y * v_up + v_forward * f_fov);
	
	vec3 v_rotation_axis = vec3(1.0, 4.0, 2.0);
	
	// Rotate the rotation axis
	mat3 m2 = Set_rot(vec3(0.1, 1.0, 0.01), control1 * 0.3);
	v_rotation_axis = v_rotation_axis * m2;
	
	float f_rotation_angle = sin(control1 * 0.5);
	
	// Mouse stuff skiped
	
	m = Set_rot(v_rotation_axis, f_rotation_angle);
	
	vec3 v_result = Trace_ray(v_camera_pos, v_rd);
	v_result = Apply_post_fx(v_result, pos);
	out_colour = vec4(v_result, 1.0);
	//out_colour = vec4(pos.x, pos.y, 0.0, 1.0);
}