#include <stdio.h>
#include "shell.h"

//sign extend :imm
uint32_t imm_sign_extend(uint32_t imm){
    int32_t signed_imm = *((int16_t*)&imm);
    uint32_t result = *((uint32_t*)&signed_imm);
    return result;
}
//sign extend :byte
uint32_t byte_sign_extend(uint8_t byte){
    int32_t signed_byte = *((int8_t*)&byte);
    uint32_t result = *((uint32_t*)&signed_byte);
    return result;
}
//sign extend :2 bytes
uint32_t bytes_sign_extend(uint16_t bytes){
    int32_t signed_bytes = *((int16_t*)&bytes);
    uint32_t result = *((uint32_t*)&signed_bytes);
    return result;
}

void process_instruction()
{
    /* execute one instuction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */

    // ensure sequntial execution
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;

    // get instuction
    uint32_t inst = mem_read_32(CURRENT_STATE.PC);
    printf("Address:%d\n",CURRENT_STATE.PC);
    printf("instuction: 0x%08x\n", inst);

    // decode
    uint32_t opcode = (inst >> 26) & 0x3F;
    // get rs | rt | rd | shamt | funct
    uint32_t rs = (inst >> 21) & 0x1F;
    uint32_t rt = (inst >> 16) & 0x1F;
    uint32_t rd = (inst >> 11) & 0x1F;
    uint32_t shamt = (inst >> 6) & 0x1F;
    uint32_t funct = inst & 0x3F;
    int32_t imm = (int32_t)(inst & 0xFFFF); // 有符号立即数
    uint32_t target = inst & 0x3FFFFFF; // 跳转目标地址
    switch(opcode){
        case 0x0:{  // R-type
            switch(funct){
                case 0x00:{  // SLL
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << shamt;
                    break;
                }
                case 0x02:{  // SRL
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> shamt;
                    break;
                }
                case 0x03:{  // SRA
                    NEXT_STATE.REGS[rd] = *((int32_t*)&CURRENT_STATE.REGS[rt]) >> shamt;
                    break;
                }
                case 0x04:{  // SLLV
                    uint32_t shamt = CURRENT_STATE.REGS[rs] & 0x1f;
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << shamt;
                    break;
                }
                case 0x06:{  // SRLV
                    uint32_t shamt = CURRENT_STATE.REGS[rs] & 0x1f;
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> shamt;
                    break;
                }
                case 0x07:{  // SRAV
                    uint32_t shamt = CURRENT_STATE.REGS[rs] & 0x1f;
                    NEXT_STATE.REGS[rd] = *((int32_t*)&CURRENT_STATE.REGS[rt]) >> shamt;
                    break;
                }
                case 0x08:{  // JR
                    NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
                }
                case 0x09:{  // JALR
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.PC + 4;
                    NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
                    break;
                }
                case 0x0c:{  // SYSCALL
                    if (CURRENT_STATE.REGS[2] == 0x0a) {
                        RUN_BIT = FALSE;
                    }
                    break;
                }
                case 0x10:{  // MFHI
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.HI;
                    break;
                }
                case 0x11:{  // MTHI
                    NEXT_STATE.HI = CURRENT_STATE.REGS[rs];
                    break;
                }
                case 0x12: {  // MFLO
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.LO;
                    break;
                }
                case 0x13: {  // MTLO
                    NEXT_STATE.LO = CURRENT_STATE.REGS[rs];
                    break;
                }
                case 0x18:{  // MULT
                    int64_t value_rs = *((int32_t*)&CURRENT_STATE.REGS[rs]);
                    int64_t value_rt = *((int32_t*)&CURRENT_STATE.REGS[rt]);
                    int64_t product = value_rs * value_rt;
                    uint64_t uint_product = *((uint32_t*)&(product));
                    NEXT_STATE.HI = (uint32_t)((uint_product >> 32) & 0xffffffff);
                    NEXT_STATE.LO = (uint32_t)(uint_product & 0xffffffff);
                    break;
                }
                case 0x19:{  // MULTH
                    uint64_t product = CURRENT_STATE.REGS[rs] * CURRENT_STATE.REGS[rt];
                    NEXT_STATE.HI = (uint32_t)((product >> 32) & 0xffffffff);
                    NEXT_STATE.LO = (uint32_t)(product & 0xffffffff);
                    break;
                }
                case 0x1a: {
                    // DIV
                    int32_t value_rs = *((int32_t*)&CURRENT_STATE.REGS[rs]);
                    int32_t value_rt = *((int32_t*)&CURRENT_STATE.REGS[rt]);
                    NEXT_STATE.LO = value_rs / value_rt;
                    NEXT_STATE.HI = value_rs % value_rt;
                    break;
                }
                case 0x1b:{  // DIVU
                    NEXT_STATE.HI = CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt];
                    NEXT_STATE.LO = CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt];
                    break;
                }
                case 0x20:{  // ADD
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] + CURRENT_STATE.REGS[rs];
                    // printf("The operands are: %d %d\n",CURRENT_STATE.REGS[rs],CURRENT_STATE.REGS[rt]);
                    // printf("The answer is :%d\n",NEXT_STATE.REGS[rd]);
                    break;
                }
                case 0x21:{  // ADDU
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] + CURRENT_STATE.REGS[rs];
                    // printf("The operands are: %d %d\n",CURRENT_STATE.REGS[rs],CURRENT_STATE.REGS[rt]);
                    // printf("The answer is :%d\n",NEXT_STATE.REGS[rd]);
                    break;
                }
                case 0x22: {  // SUB
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
                    // printf("The operands are: %d %d\n",CURRENT_STATE.REGS[rs],CURRENT_STATE.REGS[rt]);
                    // printf("The answer is :%d\n",NEXT_STATE.REGS[rd]);
                    break;
                }
                case 0x23: {  // SUBU
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
                    // printf("The operands are: %d %d\n",CURRENT_STATE.REGS[rs],CURRENT_STATE.REGS[rt]);
                    // printf("The answer is :%d\n",NEXT_STATE.REGS[rd]);
                    break;
                }
                case 0x24: {  // AND
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
                    // printf("The operands are: %d %d\n",CURRENT_STATE.REGS[rs],CURRENT_STATE.REGS[rt]);
                    // printf("The answer is :%d\n",NEXT_STATE.REGS[rd]);
                    break;
                }
                case 0x25: {  // OR
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
                    // printf("The operands are: %d %d\n",CURRENT_STATE.REGS[rs],CURRENT_STATE.REGS[rt]);
                    // printf("The answer is :%d\n",NEXT_STATE.REGS[rd]);
                    break;
                }
                case 0x26: {  // XOR
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] ^ CURRENT_STATE.REGS[rt];
                    // printf("The operands are: %d %d\n",CURRENT_STATE.REGS[rs],CURRENT_STATE.REGS[rt]);
                    // printf("The answer is :%d\n",NEXT_STATE.REGS[rd]);
                    break;
                }
                case 0x27: {  // NOR
                    NEXT_STATE.REGS[rd] = ~(CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt]);
                    // printf("The operands are: %d %d\n",CURRENT_STATE.REGS[rs],CURRENT_STATE.REGS[rt]);
                    // printf("The answer is :%d\n",NEXT_STATE.REGS[rd]);
                    break;
                }
                case 0x2a:{  //SLT
                    int32_t rs_value = *((int32_t*)&CURRENT_STATE.REGS[rs]);
                    int32_t rt_value = *((int32_t*)&CURRENT_STATE.REGS[rt]);
                    NEXT_STATE.REGS[rd] = (rs_value < rt_value) ? 1 : 0;
                    break;
                }
                case 0x2b: {  // SLTU
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt] ? 1 : 0;
                    break;
                }
                default: {
                    // printf("Unknown instruction: 0x%x\n", inst);
                    break;
                }
            }
            break;
        }
        // J-type
        case 0x02: {  // J
            // printf("J\n");
            // printf("Address_before=0x%8x\n",CURRENT_STATE.PC);
            // printf("target=%d\n",target);
            NEXT_STATE.PC = (CURRENT_STATE.PC & 0xf0000000) | (target << 2);
            // printf("Address_after=0x%8x\n",NEXT_STATE.PC);
            break;
        }
        case 0x03: {  // JAL
            // printf("JAL\n");
            // printf("Address_before=0x%8x\n",CURRENT_STATE.PC);
            // printf("target=%d\n",target);
            NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
            NEXT_STATE.PC = (CURRENT_STATE.PC & 0xf0000000) | (target << 2);
            // printf("Address_after=0x%8x\n",NEXT_STATE.PC);
            break;
        }
        // I-type
        case 0x01: {
            uint32_t offset = imm_sign_extend(imm) << 2;

            switch (rt) {
                case 0x00: {  // BLTZ
                    if ((CURRENT_STATE.REGS[rs] & 0x80000000) != 0) {
                        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                    }
                    break;
                }
                case 0x1: {  // BGEZ
                    if ((CURRENT_STATE.REGS[rs] & 0x80000000) == 0) {
                        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                    }
                    break;
                }
                case 0x10: {  // BLTZAL
                    NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
                    if ((CURRENT_STATE.REGS[rs] & 0x80000000) != 0) {
                        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                    }
                    break;
                }
                case 0x11: {  // BGEZAL
                    NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
                    if ((CURRENT_STATE.REGS[rs] & 0x80000000) == 0) {
                        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                    }
                    break;
                }
            }
            break;
        }
        case 0x08: {  // ADDI
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + imm_sign_extend(imm);
            // printf("The operands are: %d %u\n",CURRENT_STATE.REGS[rs],imm);
            // printf("The answer is :%d\n",NEXT_STATE.REGS[rt]);
            break;
        }
        case 0x09: {  // ADDIU
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + imm_sign_extend(imm);
            // printf("The operands are: %d %u\n",CURRENT_STATE.REGS[rs],imm);
            // printf("The answer is :%d\n",NEXT_STATE.REGS[rt]);
            break;
        }
        case 0x0c: {  // ANDI
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] & imm;
            // printf("The operands are: %d %u\n",CURRENT_STATE.REGS[rs],imm);
            // printf("The answer is :%d\n",NEXT_STATE.REGS[rt]);
            break;
        }
        case 0x0d: {  // ORI
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] | imm;
            // printf("The operands are: %d %u\n",CURRENT_STATE.REGS[rs],imm);
            // printf("The answer is :%d\n",NEXT_STATE.REGS[rt]);
            break;
        }
        case 0x0e: {  // XORI
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] ^ imm;
            // printf("The operands are: %d %u\n",CURRENT_STATE.REGS[rs],imm);
            // printf("The answer is :%d\n",NEXT_STATE.REGS[rt]);
            break;
        }
        case 0x04: {  // BEQ
            // printf("BEQ\n");
            // printf("Address_before=0x%8x\n",CURRENT_STATE.PC);
            // printf("target=%d\n",target);
            uint32_t offset = imm_sign_extend(imm) << 2;
            if (CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt]) {
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                // printf("Address_after=0x%8x\n",NEXT_STATE.PC);
            }
            break;
        }
        case 0x05: {  // BNE
            // printf("BNE\n");
            // printf("Address_before=0x%8x\n",CURRENT_STATE.PC);
            // printf("target=%d\n",target);
            uint32_t offset = imm_sign_extend(imm) << 2;
            if (CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt]) {
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                // printf("Address_after=0x%8x\n",NEXT_STATE.PC);
            }
            break;
        }
        case 0x06: {  // BLEZ
            uint32_t offset = imm_sign_extend(imm) << 2;
            if (rt == 0) {
                if ((CURRENT_STATE.REGS[rs] & 0x80000000) != 0 ||
                    CURRENT_STATE.REGS[rs] == 0) {
                    NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                } 
            } else {
                // Illegal instruction
                // printf("Illegal rt in BLEZ.\n");
            }
            break;
        }
        case 0x07: {  // BGTZ
            uint32_t offset = imm_sign_extend(imm) << 2;
            if (rt == 0) {
                if ((CURRENT_STATE.REGS[rs] & 0x80000000) == 0 &&
                    CURRENT_STATE.REGS[rs] != 0) {
                    NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                }
            } else {
                // Illegal instruction
                // printf("Illegal rt in BGTZ.\n");
            }
            break;
        }
        case 0x0f: {  // LUI
            NEXT_STATE.REGS[rt]  = (CURRENT_STATE.REGS[rt] & 0x0000ffff) | (imm << 16);
            break;
        }
        case 0x20: {  // LB
            uint32_t addr = imm_sign_extend(imm) + CURRENT_STATE.REGS[rs];
            uint8_t byte = mem_read_32(addr) & 0xff;
            NEXT_STATE.REGS[rt] = byte_sign_extend(byte);
            break;
        }
        case 0x24: {  // LBU
            uint32_t addr = imm_sign_extend(imm) + CURRENT_STATE.REGS[rs];
            uint8_t byte = mem_read_32(addr) & 0xff;
            NEXT_STATE.REGS[rt] = byte;
            break;
        }
        case 0x21: {  // LH
            uint32_t addr = imm_sign_extend(imm) + CURRENT_STATE.REGS[rs];
            uint16_t bytes = mem_read_32(addr) & 0xffff;
            NEXT_STATE.REGS[rt] = bytes_sign_extend(bytes);            break;
        }
        case 0x25: {  // LHU
            uint32_t addr = imm_sign_extend(imm) + CURRENT_STATE.REGS[rs];
            uint16_t bytes = mem_read_32(addr) & 0xffff;
            NEXT_STATE.REGS[rt] = bytes;
            break;
        }
        case 0x23: {  // LW
            uint32_t addr = imm_sign_extend(imm) + CURRENT_STATE.REGS[rs];
            NEXT_STATE.REGS[rt] = mem_read_32(addr);
            break;
        }
        case 0x28: {  // SB
            uint32_t addr = imm_sign_extend(imm) + CURRENT_STATE.REGS[rs];
            uint32_t val = (mem_read_32(addr) & 0xffffff00) | (CURRENT_STATE.REGS[rt] & 0xff);
            mem_write_32(addr, val);
            break;
        }
        case 0x29: {  // SH
            uint32_t addr = imm_sign_extend(imm) + CURRENT_STATE.REGS[rs];
            uint32_t val = (mem_read_32(addr) & 0xffff0000) | (CURRENT_STATE.REGS[rt] & 0xffff);
            mem_write_32(addr, val);
            break;
        }
        case 0x2b: {  // SW
            uint32_t addr = imm_sign_extend(imm) + CURRENT_STATE.REGS[rs];
            mem_write_32(addr, CURRENT_STATE.REGS[rt]);
            break;
        }
        default: {
            printf("unimplemented instruction: 0x%08x\n", inst);
            break;
        }
    }
}
