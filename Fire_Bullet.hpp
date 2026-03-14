void* Fire_Bullet_Target;

void __thiscall Fire_Bullet(void* Entity)
{
    if (Entity == (void*)((unsigned __int32)Fire_Bullet_Target ^ 1))
    {
        Fire_Bullet_Target = Entity;
    }
}

void* Original_Fire_Bullet_Caller;

void __declspec(naked) Redirected_Fire_Bullet()
{
    __asm
    {
        pushad
        mov ecx, esi
        call Fire_Bullet
        popad
        push Original_Fire_Bullet_Caller
        ret
    }
}
