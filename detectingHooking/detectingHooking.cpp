#include <iostream>
#include <Windows.h>

uint8_t *get_nextAddrFromIndirectJmp(uint8_t* inst_ptr, int length_of_current_instruction)
{
    //Ensure the instruction begin with a prefix REX
    uint8_t rex_prefix = (inst_ptr[0] >> 4) & 15;
    if (rex_prefix != 4) {
        fprintf(stderr,"Error. The instruction does not begin with a prefix\n");
    }  

    if (inst_ptr[1] != 0xff) {
        fprintf(stderr, "Error. This does not seems to be a jmp or call as we expect\n");
    }

    //Ensure ModR/M
    uint8_t mod = (inst_ptr[2] >> 6) & 3;
    uint8_t reg = (inst_ptr[2] >> 3) & 7;
    uint8_t rm = inst_ptr[2] & 7;
    if (rm != 5) {
        fprintf(stderr, "Error. The instruction does not use a disp32 (32bit offset)\n");
    }


    DWORD* offset_ptr = (DWORD*)(inst_ptr + 3);
    uint64_t ptr = (uint64_t)inst_ptr + *offset_ptr + length_of_current_instruction;
    return *(uint8_t**)ptr;
}
int main()
{
    HMODULE libraryBase = LoadLibraryA("kernel32.dll");
    uint8_t* current_ptr = (uint8_t*)GetProcAddress(libraryBase, "ReadProcessMemory");

    // At this stage, current_ptr point to the memory that contains the following instructions:
    // 48:FF25 816B0600 jmp qword ptr ds:[<ReadProcessMemory]
    current_ptr = get_nextAddrFromIndirectJmp(current_ptr, 7);


    // At this stage, current_ptr point to the memory that contains the following instructions:
    // 48:83EC 48          sub rsp,48
    // 48:8D4424 30        lea rax,qword ptr ss:[rsp+30]
    // 48:894424 20        mov qword ptr ss:[rsp+20],rax
    // 48:FF15 ABFF1E00    call qword ptr ds:[<NtReadVirtualMemory>]
    current_ptr += 14; // go to the call instruction
    current_ptr = get_nextAddrFromIndirectJmp(current_ptr, 7);

    // At this stage, current_ptr point to the memory that contains the following instructions:
    // 4C:8BD1             mov r10,rcx
    // B8 3F000000         mov eax, 3F
    // F60425 0803FE7F 01  test byte ptr ds:[7FFE0308],1
    // 75 03               jne ntdll.7FFBD55443E5
    // 0F05                syscall
    printf("%02x (should be 0x4C for unhooked)\n", current_ptr[0]);




}
