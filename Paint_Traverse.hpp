#pragma once

#include "vmt.hpp"

#include "interfaces.hpp"

#include "global_utils.hpp"

#include "matrix.hpp"

unsigned long Esp_Font;
unsigned long Esp_Font1;
unsigned long Indicator_Font;


bool world2screen(const vec3_t& pos, vec3_t& output)
{
	VMatrix matrix = *(VMatrix*)(*(int(__thiscall**)(DWORD))(**(DWORD***)540447280 + 15))(*(DWORD*)540447280); // *(VMatrix*)540447280;

	output.x = matrix[0][0] * pos.x + matrix[0][1] * pos.y + matrix[0][2] * pos.z + matrix[0][3];
	output.y = matrix[1][0] * pos.x + matrix[1][1] * pos.y + matrix[1][2] * pos.z + matrix[1][3];

	float w = matrix[3][0] * pos.x + matrix[3][1] * pos.y + matrix[3][2] * pos.z + matrix[3][3];

	if (w < 0.01f)
		return false;

	float inv_w = 1.0f / w;

	output.x *= inv_w;
	output.y *= inv_w;

	using Get_Screen_Size_Type = __int32(__cdecl*)();

	int True_Screen_Width = Get_Screen_Size_Type(604542880)();

	int True_Screen_Height = Get_Screen_Size_Type(604542848)();


	float x = (float)(True_Screen_Width / 2);
	float y = (float)(True_Screen_Height / 2);


	x += 0.5f * output.x * True_Screen_Width + 0.5f;
	y -= 0.5f * output.y * True_Screen_Height + 0.5f;

	output.x = x;
	output.y = y;

	return true;
}


const char* get_panel_name(unsigned int vgui_panel)
{
	typedef const char* (__thiscall* o_fn)(void*, unsigned int);
	return global_utils::v_function<o_fn>(interfaces::vgui_ipanel, 35)(interfaces::vgui_ipanel, vgui_panel);
}

/*
* flags
* 0 - align left
* 1 - right
* 2 - x center
* 3 - y center
*/
void Draw_Text(unsigned long Font, __int32 Flags, __int32 X, __int32 Y, __int32 Red, __int32 Green, __int32 Blue, __int32 Alpha, char* String, ...)
{
	void* Surface = *(void**)608279384;

	va_list VaList;
	char Buffer[1024];
	wchar_t WBuffer[1024];

	va_start(VaList, String);
	vsprintf(Buffer, String, VaList);
	va_end(VaList);

	MultiByteToWideChar(CP_UTF8, 0, Buffer, 256, WBuffer, 256);

	using Draw_Set_Text_Font_Type = void(__thiscall**)(void* Surface, unsigned long Font);

	(*Draw_Set_Text_Font_Type(*(unsigned __int32*)Surface + 68))(Surface, Esp_Font);

	__int32 Wide, Tall;

	using Get_Text_Size_Type = void(__thiscall**)(void* Surface, unsigned long Font, wchar_t* Text, __int32& Wide, __int32& Tall);

	(*Get_Text_Size_Type(*(unsigned __int32*)Surface + 288))(Surface, Esp_Font, WBuffer, Wide, Tall);

	if (Flags & 1)
		X -= Wide;
	if (Flags & 2)
		X -= Wide / 2;
	if (Flags & 3)
		Y -= Tall / 2;

	using Draw_Set_Text_Color_Type = void(__thiscall**)(void* Surface, __int32 Red, __int32 Green, __int32 Blue, __int32 Alpha);

	(*Draw_Set_Text_Color_Type(*(unsigned __int32*)Surface + 76))(Surface, Red, Green, Blue, Alpha);

	using Draw_Set_Text_Pos_Type = void(__thiscall**)(void* Surface, __int32 X, __int32 Y);

	(*Draw_Set_Text_Pos_Type(*(unsigned __int32*)Surface + 80))(Surface, X, Y);

	using Draw_Print_Text_Type = void(__thiscall**)(void* Surface, wchar_t* Text, __int32 Length, __int32 Unknown1);

	(*Draw_Print_Text_Type(*(unsigned __int32*)Surface + 88))(Surface, WBuffer, wcslen(WBuffer), 0);
}


void draw_esp()
{
	void* Local_Player = *(void**)607867332;

	if (!Local_Player)
		return;

	void* Surface = *(void**)608279384;

	using Set_Color_Type = void(__thiscall**)(void* Surface, unsigned __int8 Red, unsigned __int8 Green, unsigned __int8 Blue, unsigned __int8 Alpha);

	using Draw_Filled_Rect_Type = void(__thiscall**)(void* Surface, int x01, int y01, int x02, int y02);


	for (int Entity_Number = 1; Entity_Number < (*(Global_Variables_Structure**)607726732)->Maximum_Clients; ++Entity_Number) {
		void* Entity = *(void**)((unsigned __int32)607973860 + ((Entity_Number - 4097) << 4));

		if (!Entity || *(__int8*)((unsigned __int32)Entity + 135) != 0)
			continue;

		if (*(__int32*)((unsigned __int32)Entity + 144) == *(__int32*)((unsigned __int32)Local_Player + 144))
			continue;

		if (*(__int8*)((unsigned __int32)Entity + 320) != 0)
			continue;


		Player_Data_Structure* Player_Data = &Players_Data[Entity_Number];

		if (!Player_Data)
			continue;

		__int32 health = *reinterpret_cast<__int32*>((unsigned __int32)Entity + 3492);

		if (health <= 0 || health > 150)
			continue;


		if (Entity && Entity != Local_Player) {

			vec3_t* Entity_Origin = reinterpret_cast<vec3_t*>((unsigned __int32)Entity + 668);

			vec3_t bottom_position, top_position;

			vec3_t copy = *Entity_Origin;


			if (!world2screen(*Entity_Origin, bottom_position) || !world2screen(copy + vec3_t(0, 0, 72), top_position))
				continue;

			float height = bottom_position.y - top_position.y;
			float half_width = height * .31f;

			using Get_Name_Type = void(__thiscall*)(void* Engine, __int32 Number, char* Name);

			char Name[132];

			Get_Name_Type(537018208)((void*)540435380, Entity_Number, Name);

			Draw_Text(Esp_Font, 2, (top_position.x - half_width) + half_width, top_position.y - 8, 255, 255, 255, 255, Name);

			(*Set_Color_Type(*(unsigned __int32*)Surface + 44))(Surface, 255, 255, 255, 255);

			(*Draw_Filled_Rect_Type(*(unsigned __int32*)Surface + 56))(Surface, top_position.x - half_width, top_position.y, bottom_position.x + half_width, bottom_position.y);



			(*Set_Color_Type(*(unsigned __int32*)Surface + 44))(Surface, 0, 0, 0, 255);

			(*Draw_Filled_Rect_Type(*(unsigned __int32*)Surface + 56))(Surface, top_position.x - half_width - 1, top_position.y - 1, bottom_position.x + half_width + 1, bottom_position.y + 1);

			(*Set_Color_Type(*(unsigned __int32*)Surface + 44))(Surface, 0, 0, 0, 255);

			(*Draw_Filled_Rect_Type(*(unsigned __int32*)Surface + 56))(Surface, top_position.x - half_width + 1, top_position.y + 1, bottom_position.x + half_width - 1, bottom_position.y - 1);


			using Get_Weapon_Type = void* (__thiscall*)(void* Entity);

			void* Weapon = Get_Weapon_Type(604036352)(Entity);

			if (!Weapon)
				continue;

			using Get_Weapon_Name_Type = char* (__thiscall**)(void* Weapon);

			Draw_Text(Esp_Font, 2, (top_position.x - half_width) + half_width, bottom_position.y + 7, 255, 255, 255, 255, (*Get_Weapon_Name_Type(*(unsigned __int32*)Weapon + 1040))(Weapon) + 7);


			if (health > 0)
			{
				int Scale = health * 2.55f;
				auto iSizeH = (int)std::round(((height)*health) / 100);
				auto iRealH = height - iSizeH;
				//outline
				(*Set_Color_Type(*(unsigned __int32*)Surface + 44))(Surface, 0, 0, 0, 255);
				(*Draw_Filled_Rect_Type(*(unsigned __int32*)Surface + 56))(Surface, top_position.x - half_width - 6, top_position.y + iRealH, top_position.x - half_width - 5 + 4, top_position.y + iRealH + iSizeH);
				//green
				(*Set_Color_Type(*(unsigned __int32*)Surface + 44))(Surface, 0, 225, 0, 255);
				(*Draw_Filled_Rect_Type(*(unsigned __int32*)Surface + 56))(Surface, top_position.x - half_width - 5, top_position.y + iRealH, top_position.x - half_width - 5 + 2, top_position.y + iRealH + iSizeH);
				
			}
		}
	}
}

typedef void(__thiscall* paint_traverse_t) (void*, unsigned int, bool, bool);

void __thiscall Redirected_Paint_Traverse(void* thisptr, unsigned int vgui_panel, bool force_repaint, bool allow_force)
{
	static auto o_paint_traverse = panel_hook->get_original<paint_traverse_t>(40);

	o_paint_traverse(thisptr, vgui_panel, force_repaint, allow_force);

	const char* panel_name = get_panel_name(vgui_panel);

	if (panel_name && panel_name[0] == 'M' && panel_name[3] == 'S' && panel_name[9] == 'T')
	{
		draw_esp(); //# draw esp

		void* Local_Player = *(void**)607867332;

		if (Local_Player)
		{
			float* Velocity = (float*)((unsigned __int32)Local_Player + 224);

			float Velocity_Corrected = __builtin_sqrtf(Velocity[0] * Velocity[0] + Velocity[1] * Velocity[1] + Velocity[2] * Velocity[2]);

			Draw_Text(Indicator_Font, 0, 5, 700, 255, 255, 255, 255, (char*)"%0.f VEL", Velocity_Corrected);
		}
	}
}

