void Redirected_Draw_Crosshair()
{

    void* Surface = *(void**)608279384;

    using Set_Color_Type = void(__thiscall**)(void* Surface, unsigned __int8 Red, unsigned __int8 Green, unsigned __int8 Blue, unsigned __int8 Alpha);

    using Draw_Filled_Rect_Type = void(__thiscall**)(void* Surface, int x01, int y01, int x02, int y02);
    using Get_Screen_Size_Type = __int32(__cdecl*)();

    __int32 x = Get_Screen_Size_Type(604542880)() / 2;

    __int32 y = Get_Screen_Size_Type(604542848)() / 2;

    (*Set_Color_Type(*(unsigned __int32*)Surface + 44))(Surface, 0, 0, 0, 255);
    (*Draw_Filled_Rect_Type(*(unsigned __int32*)Surface + 56))(Surface, x - 5, y - 1, x - 5 + 11, y - 1 + 3);

    (*Set_Color_Type(*(unsigned __int32*)Surface + 44))(Surface, 0, 0, 0, 255);
    (*Draw_Filled_Rect_Type(*(unsigned __int32*)Surface + 56))(Surface, x - 1, y - 5, x - 1 + 3, y - 5 + 11);

    (*Set_Color_Type(*(unsigned __int32*)Surface + 44))(Surface, 255, 255, 255, 255);
    (*Draw_Filled_Rect_Type(*(unsigned __int32*)Surface + 56))(Surface, x - 4, y, x - 4 + 9, y + 1);

    (*Set_Color_Type(*(unsigned __int32*)Surface + 44))(Surface, 255, 255, 255, 255);
    (*Draw_Filled_Rect_Type(*(unsigned __int32*)Surface + 56))(Surface, x, y - 4, x + 1, y - 4 + 9);

}