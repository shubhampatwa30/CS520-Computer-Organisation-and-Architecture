/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <string.h>
#include "stagelist.h" //Reference : https://www.zentut.com/c-tutorial/c-linked-list/

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
    case OPCODE_STR:
    case OPCODE_ADD:
    case OPCODE_SUB:
    case OPCODE_MUL:
    case OPCODE_DIV:
    case OPCODE_AND:
    case OPCODE_OR:
    case OPCODE_XOR:
    case OPCODE_LDR:
    {
        printf("%s,R%d,R%d,R%d", stage->opcode_str, stage->rd, stage->rs1,
               stage->rs2);
        break;
    }

    case OPCODE_MOVC:
    {
        printf("%s,R%d,#%d", stage->opcode_str, stage->rd, stage->imm);
        break;
    }

    case OPCODE_LOAD:
    case OPCODE_ADDL:
    case OPCODE_SUBL:
    case OPCODE_JAL:
    {
        printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
               stage->imm);
        break;
    }

    case OPCODE_STORE:
    {
        printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
               stage->imm);
        break;
    }

    case OPCODE_BZ:
    case OPCODE_BNZ:
    {
        printf("%s,#%d ", stage->opcode_str, stage->imm);
        break;
    }
    case OPCODE_JUMP:
    {
        printf("%s R%d,#%d ", stage->opcode_str, stage->rs1, stage->imm);
        break;
    }

    case OPCODE_CMP:
    {
        printf("%s R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2);
        break;
    }

    case OPCODE_HALT:
    {
        printf("%s", stage->opcode_str);
        break;
    }

    case OPCODE_NULL:
    {
        printf(" ");
        break;
    }
    case OPCODE_NOP:
        printf("%s", stage->opcode_str);
    }
}

static void
print_instruction_with_renamed_registers(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
    case OPCODE_STR:
    case OPCODE_ADD:
    case OPCODE_SUB:
    case OPCODE_MUL:
    case OPCODE_DIV:
    case OPCODE_AND:
    case OPCODE_OR:
    case OPCODE_XOR:
    case OPCODE_LDR:
    {
        printf("%s,R%d,R%d,R%d\t\t%s,P%d,P%d,P%d", stage->opcode_str, stage->rd, stage->rs1,
               stage->rs2, stage->opcode_str, stage->pd, stage->ps1, stage->ps2);
        break;
    }

    case OPCODE_MOVC:
    {
        printf("%s,R%d,#%d\t\t%s,P%d,#%d", stage->opcode_str, stage->rd, stage->imm, stage->opcode_str, stage->pd, stage->imm);
        break;
    }

    case OPCODE_LOAD:
    case OPCODE_ADDL:
    case OPCODE_SUBL:
    case OPCODE_JAL:
    {
        printf("%s,R%d,R%d,#%d\t\t%s,P%d,P%d,#%d", stage->opcode_str, stage->rd, stage->rs1,
               stage->imm, stage->opcode_str, stage->pd, stage->ps1, stage->imm);
        break;
    }

    case OPCODE_STORE:
    {
        printf("%s,R%d,R%d,#%d\t%s,P%d,P%d,#%d", stage->opcode_str, stage->rs1, stage->rs2,
               stage->imm, stage->opcode_str, stage->ps1, stage->ps2, stage->imm);
        break;
    }

    case OPCODE_BZ:
    case OPCODE_BNZ:
    {
        printf("%s,#%d", stage->opcode_str, stage->imm);
        break;
    }

    case OPCODE_CMP:
    {
        printf("%s R%d,R%d\t\t%s P%d,P%d", stage->opcode_str, stage->rs1, stage->rs2, stage->opcode_str, stage->ps1, stage->ps2);
        break;
    }

    case OPCODE_HALT:
    {
        printf("%s", stage->opcode_str);
        break;
    }
    case OPCODE_JUMP:
    {
        printf("%s R%d,#%d\t\t%s P%d,#%d", stage->opcode_str, stage->rs1, stage->imm, stage->opcode_str, stage->ps1, stage->imm);
        break;
    }

    case OPCODE_NULL:
    {
        printf(" ");
        break;
    }
    case OPCODE_NOP:
        printf("%s", stage->opcode_str);
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    //print_instruction(stage);
    print_instruction_with_renamed_registers(stage);

    printf("\n");
}
/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content_for_fetch(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if (cpu->decode.stalled == 1)
    {
        cpu->fetch.stalled = 1;
    }
    else
    {
        cpu->fetch.stalled = 0;
    }

    if (cpu->fetch.flush == 1)
    {
        strcpy(cpu->fetch.opcode_str, " ");
        cpu->fetch.opcode = 0x0;
        cpu->fetch.pc = '\0';
        cpu->fetch.flush = 0;
        return;
    }

    /* This fetches new branch target instruction from next cycle */
    if (cpu->fetch_from_next_cycle == TRUE)
    {
        cpu->fetch_from_next_cycle = FALSE;

        /* Skip this cycle*/
        return;
    }

    /* Store current PC in fetch latch */
    cpu->fetch.pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields
         * into fetch latch  */
    current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
    strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
    cpu->fetch.opcode = current_ins->opcode;
    cpu->fetch.rd = current_ins->rd;
    cpu->fetch.rs1 = current_ins->rs1;
    cpu->fetch.rs2 = current_ins->rs2;
    cpu->fetch.imm = current_ins->imm;
    if (cpu->fetch.opcode == 0)
    {
        strcpy(cpu->fetch.opcode_str, " ");
        cpu->fetch.pc = '\0';
    }

    if (cpu->fetch.has_insn && (!cpu->fetch.stalled))
    {
        /* Update PC for next instruction */
        cpu->pc += 4;

        /* Copy data from fetch latch to decode latch*/
        if (cpu->decode.stalled == 0)
        {
            cpu->decode = cpu->fetch;
        }
    }

    if (ENABLE_DEBUG_MESSAGES && cpu->fetch.opcode != OPCODE_NULL)
    {
        print_stage_content_for_fetch("Fetch", &cpu->fetch);
    }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{

    if (cpu->decode.flush == 1)
    {
        strcpy(cpu->decode.opcode_str, " ");
        cpu->decode.opcode = 0x0;
        cpu->decode.pc = 0000;
        cpu->decode.flush = 0;
    }
    if (cpu->decode.stalled == 1)
    {
        cpu->issueq.flush = 1;
        cpu->rob.flush =1;
    }
    else
    {
        cpu->issueq.flush = 0;
        cpu->rob.flush =0;
    }

    if (cpu->decode.has_insn && (!cpu->decode.stalled))
    {
        cpu->decode.instype = 0;
        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {

        case OPCODE_STR:
        {
            cpu->decode.ps1 = searchprftop(rfprf, cpu->decode.rs1);
            cpu->decode.ps2 = searchprftop(rfprf, cpu->decode.rs2);
            cpu->decode.pd = searchprftop(rfprf, cpu->decode.rd);

            if (cpu->pregs_valid[cpu->decode.ps1] && cpu->pregs_valid[cpu->decode.ps2] && cpu->pregs_valid[cpu->decode.pd])
            {
                cpu->mem_valid[cpu->renameTableValues[cpu->decode.ps1] + cpu->renameTableValues[cpu->decode.ps2]] = 0;
                cpu->decode.instype = 1;
                cpu->mreadybit[cpu->decode.pc] = 1;
            }

            break;
        }

        case OPCODE_STORE:
        {
            cpu->decode.ps1 = searchprftop(rfprf, cpu->decode.rs1);
            cpu->decode.ps2 = searchprftop(rfprf, cpu->decode.rs2);
            // cpu->decode.pd = searchprftop(rfprf, cpu->decode.rd);

            if (cpu->pregs_valid[cpu->decode.ps1] && cpu->pregs_valid[cpu->decode.ps2])
            {
                cpu->mem_valid[cpu->decode.imm + cpu->renameTableValues[cpu->decode.ps2]] = 0;
                cpu->decode.instype = 1;
                cpu->mreadybit[cpu->decode.pc] = 1;
            }

            break;
        }

        case OPCODE_LDR:
        {

            cpu->decode.ps1 = searchprftop(rfprf, cpu->decode.rs1);
            cpu->decode.ps2 = searchprftop(rfprf, cpu->decode.rs2);
            cpu->decode.pd = phead->data;
            cpu->pregs_valid[cpu->decode.pd] = 0;
            renametable.rf_code = cpu->decode.rd;
            renametable.prf_code = cpu->decode.pd;
            rfprf = prependIntoRenameTable(rfprf, renametable);
            phead = dequeueReg(phead);

            if (cpu->pregs_valid[cpu->decode.ps1] && cpu->pregs_valid[cpu->decode.ps2])
            {
                cpu->decode.instype = 1;
                cpu->mreadybit[cpu->decode.pc] = 1;
            }

            break;
        }

        case OPCODE_LOAD:
        {

            cpu->decode.ps1 = searchprftop(rfprf, cpu->decode.rs1);
            cpu->decode.pd = phead->data;
            cpu->pregs_valid[cpu->decode.pd] = 0;
            renametable.rf_code = cpu->decode.rd;
            renametable.prf_code = cpu->decode.pd;
            rfprf = prependIntoRenameTable(rfprf, renametable);
            phead = dequeueReg(phead);

            if (cpu->pregs_valid[cpu->decode.ps1])
            {
                cpu->decode.instype = 1;
                cpu->mreadybit[cpu->decode.pc] = 1;
            }

            break;
        }

        case OPCODE_CMP:
        {
            cpu->decode.ps1 = searchprftop(rfprf, cpu->decode.rs1);
            cpu->decode.ps2 = searchprftop(rfprf, cpu->decode.rs2);
            break;
        }

        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        {
            
            cpu->decode.ps1 = searchprftop(rfprf, cpu->decode.rs1);
            cpu->decode.ps2 = searchprftop(rfprf, cpu->decode.rs2);
            cpu->decode.pd = phead->data;
            cpu->pregs_valid[cpu->decode.pd] = 0;
            renametable.rf_code = cpu->decode.rd;
            renametable.prf_code = cpu->decode.pd;
            rfprf = prependIntoRenameTable(rfprf, renametable);
            phead = dequeueReg(phead);
            

            break;
        }
   

        case OPCODE_ADDL:
        case OPCODE_SUBL:
        case OPCODE_JAL:
        {
            cpu->decode.ps1 = searchprftop(rfprf, cpu->decode.rs1);
            cpu->decode.pd = phead->data;
            cpu->pregs_valid[cpu->decode.pd] = 0;
            renametable.rf_code = cpu->decode.rd;
            renametable.prf_code = cpu->decode.pd;
            rfprf = prependIntoRenameTable(rfprf, renametable);
            phead = dequeueReg(phead);

            break;
        }

        case OPCODE_MOVC:
        {

            cpu->decode.pd = phead->data;
            renametable.rf_code = cpu->decode.rd;
            renametable.prf_code = cpu->decode.pd;
            cpu->pregs_valid[cpu->decode.pd] = 0;
            rfprf = prependIntoRenameTable(rfprf, renametable);
            phead = dequeueReg(phead);

            break;
        }

        case OPCODE_BNZ:
        case OPCODE_BZ:
        {

            break;
        }

        case OPCODE_JUMP:
        {
            cpu->decode.ps1 = searchprftop(rfprf, cpu->decode.rs1);
            break;
        }

        case OPCODE_HALT:
        {
            //cpu->fetch.flush = 1;

            break;
        }
        default:
        {
            break;
        }
        }
    }

    /* Copy data from decode latch to intfu latch*/
    if (cpu->decode.stalled == 0)
    {
        if (cpu->decode.instype == 0)
        {
            if(count(iqhead) < 23 && count(robhead) < 64){
            cpu->issueq = cpu->decode;
            cpu->rob = cpu->decode;}
            else{
                cpu->decode.stalled = 1;
            }
        }
        else
        {
            if(count(robhead) < 64){
           
            cpu->rob = cpu->decode;}
            else{
                cpu->decode.stalled = 1;
            }
        }
        cpu->decode.has_insn = FALSE;
    }

    if (ENABLE_DEBUG_MESSAGES && cpu->decode.opcode != OPCODE_NULL)
    {
        print_stage_content("Decode/RF", &cpu->decode);
    }
}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_issueq(APEX_CPU *cpu)
{

    /**
     * 
     * Issue queue is deployed as a list.
     * The decode instruction is appended to the issue queue( from the last part);
     * 
     */
    if (cpu->issueq.flush == 1)
    {
        strcpy(cpu->issueq.opcode_str, " ");
        cpu->issueq.opcode = 0x0;
        cpu->issueq.pc = 0000;
        cpu->issueq.flush = 0;
    }

    if (cpu->issueq.opcode != 0x0 && cpu->issueq.has_insn == TRUE)
    {
        iqhead = enqueue(iqhead, cpu->issueq);
    }
    if (cpu->issueq.opcode == 0xc)
    {
        cpu->decode.flush = 1;
    }

    node *cursor = iqhead;
    while (cursor != NULL)
    {
        if (ENABLE_DEBUG_MESSAGES && cursor->data.opcode != OPCODE_NULL)
        {
            print_stage_content("Issueq", &cursor->data);
        }
        cursor = cursor->next;
    }
    int intfuBusyFlag = 0;
    int mulfuBusyFlag = 0;
    int jbuBusyFlag = 0;
    

    if (count(iqhead) != 0)
    {

        node *cursor = iqhead;

        while (cursor != NULL)
        {
            //printf("opcode : %s\n ",cursor->data.opcode_str);
            switch (cursor->data.opcode)
            {

            case OPCODE_STR:
            {

                if (cpu->pregs_valid[cursor->data.ps1] && cpu->pregs_valid[cursor->data.ps2] && cpu->pregs_valid[cursor->data.pd])
                {

                    // printf("PS! : %d",cpu->renameTableValues[cursor->data.ps1]);
                    // printf("PS2 : %d",cpu->renameTableValues[cursor->data.ps2]);
                    cpu->mem_valid[cpu->renameTableValues[cursor->data.ps1] + cpu->renameTableValues[cursor->data.ps2]] = 0;
                    cpu->mreadybit[cursor->data.pc] = 1;
                    iqhead = remove_any(iqhead, cursor);
                }
                break;
            }

            case OPCODE_STORE:
            {
                if (cpu->pregs_valid[cursor->data.ps1] && cpu->pregs_valid[cursor->data.ps2])
                {
                    cpu->mem_valid[cpu->renameTableValues[cursor->data.ps2] + cpu->renameTableValues[cursor->data.imm]] = 0;
                    cpu->mreadybit[cursor->data.pc] = 1;
                    iqhead = remove_any(iqhead, cursor);
                }
                break;
            }

            case OPCODE_LDR:
            {
                if (cpu->pregs_valid[cursor->data.ps1] && cpu->pregs_valid[cursor->data.ps2])
                {
                    cpu->pregs_valid[cursor->data.pd] = 0;
                    cpu->mreadybit[cursor->data.pc] = 1;
                    iqhead = remove_any(iqhead, cursor);
                }
                break;
            }

            case OPCODE_LOAD:
            {
                if (cpu->pregs_valid[cursor->data.ps1])
                {
                    cpu->pregs_valid[cursor->data.pd] = 0;
                    cpu->mreadybit[cursor->data.pc] = 1;
                    iqhead = remove_any(iqhead, cursor);
                }
                break;
            }

            case OPCODE_CMP:
            {
                if (cpu->pregs_valid[cursor->data.ps1] && cpu->pregs_valid[cursor->data.ps2] && intfuBusyFlag != 1)
                {
                    cursor->data.ps1_value = cpu->renameTableValues[cursor->data.ps1];
                    cursor->data.ps2_value = cpu->renameTableValues[cursor->data.ps2];
                    cpu->intfu = cursor->data;
                    //printf(" IN here %s",cursor->data.opcode_str);
                    iqhead = remove_any(iqhead, cursor);
                    // tmp = enqueue(tmp,cursor->data);
                    intfuBusyFlag = 1;
                }

                break;
            }

            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            {
                if (cpu->pregs_valid[cursor->data.ps1] && cpu->pregs_valid[cursor->data.ps2] && intfuBusyFlag != 1)
                {
                    cursor->data.ps1_value = cpu->renameTableValues[cursor->data.ps1];
                    cursor->data.ps2_value = cpu->renameTableValues[cursor->data.ps2];
                    cpu->pregs_valid[cursor->data.pd] = 0;
                    cpu->intfu = cursor->data;
                    iqhead = remove_any(iqhead, cursor);
                    //tmp = enqueue(tmp,cursor->data);
                    intfuBusyFlag = 1;
                }

                break;
            }

            case OPCODE_JUMP:
            {
                if (cpu->pregs_valid[cursor->data.ps1] && jbuBusyFlag != 1)
                {
                    cursor->data.ps1_value = cpu->renameTableValues[cursor->data.ps1];
                    cpu->jbu1 = cursor->data;
                    iqhead = remove_any(iqhead, cursor);
                    //tmp = enqueue(tmp,cursor->data);
                    jbuBusyFlag = 1;
                }

                break;
            }

            case OPCODE_JAL:
            {
                if (cpu->pregs_valid[cursor->data.ps1] && jbuBusyFlag != 1)
                {
                    cursor->data.ps1_value = cpu->renameTableValues[cursor->data.ps1];
                    cpu->pregs_valid[cursor->data.pd] = 0;
                    cpu->jbu1 = cursor->data;

                    iqhead = remove_any(iqhead, cursor);
                    //tmp = enqueue(tmp,cursor->data);
                    jbuBusyFlag = 1;
                }

                break;
            }
            case OPCODE_BNZ:
            case OPCODE_BZ:
            {
                if (jbuBusyFlag != 1)
                {
                    cpu->jbu1 = cursor->data;
                    iqhead = remove_any(iqhead, cursor);
                    jbuBusyFlag = 1;
                }
                break;
            }

            case OPCODE_MUL:
            {
                //printf("MUL STILL IN ISSUEQ\n");
                if (cpu->pregs_valid[cursor->data.ps1] && cpu->pregs_valid[cursor->data.ps2] && mulfuBusyFlag != 1)
                {
                    cursor->data.ps1_value = cpu->renameTableValues[cursor->data.ps1];
                    cursor->data.ps2_value = cpu->renameTableValues[cursor->data.ps2];
                    cpu->pregs_valid[cursor->data.pd] = 0;
                    cpu->mulfu = cursor->data;
                    iqhead = remove_any(iqhead, cursor);
                    //tmp = enqueue(tmp,cursor->data);
                    mulfuBusyFlag = 1;
                }

                break;
            }
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            {
                if (cpu->pregs_valid[cursor->data.ps1] && intfuBusyFlag != 1)
                {
                    cursor->data.ps1_value = cpu->renameTableValues[cursor->data.ps1];
                    cpu->pregs_valid[cursor->data.pd] = 0;
                    cpu->intfu = cursor->data;
                    iqhead = remove_any(iqhead, cursor);
                    //tmp = enqueue(tmp,cursor->data);
                    intfuBusyFlag = 1;
                }

                break;
            }

            case OPCODE_MOVC:
            {
                /* MOVC doesn't have register operands */
                if (intfuBusyFlag != 1)
                {
                    cpu->intfu = cursor->data;
                    cpu->pregs_valid[cursor->data.pd] = 0;
                    iqhead = remove_any(iqhead, cursor);
                    //tmp = enqueue(tmp,cursor->data);
                    intfuBusyFlag = 1;
                }
                break;
            }

            // case OPCODE_HALT:
            // {
            //     cpu->fetch.flush = 1;
            //     break;
            // }
            default:
            {
                 iqhead = remove_any(iqhead, cursor);
                break;
            }
            }
            cursor = cursor->next;
        }
    }
    cpu->decode.stalled = 0;
    cpu->issueq.has_insn = FALSE;
}
/*
 * INT FU Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_intfu(APEX_CPU *cpu)
{
    if (cpu->intfu.flush == 1)
    {
        strcpy(cpu->intfu.opcode_str, " ");
        cpu->intfu.opcode = 0x0;
        cpu->intfu.pc = 0000;
    }
    if (cpu->intfu.has_insn)
    {
        /* intfu logic based on instruction type */
        switch (cpu->intfu.opcode)
        {
        case OPCODE_CMP:
        {

            cpu->intfu.result_buffer = cpu->intfu.ps1_value - cpu->intfu.ps2_value;
            cpu->cmpvalue[cpu->intfu.pc] = cpu->intfu.result_buffer;
            cpu->cmp_completed = 1;
            break;
        }

        case OPCODE_ADD:
        {

            cpu->intfu.result_buffer = cpu->intfu.ps1_value + cpu->intfu.ps2_value;
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;  
            cpu->pregs_valid[cpu->intfu.pd] = 1;
            break;
        }
        case OPCODE_SUB:
        {
            cpu->intfu.result_buffer = cpu->intfu.ps1_value - cpu->intfu.ps2_value;
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;
            cpu->pregs_valid[cpu->intfu.pd] = 1;

            break;
        }

        case OPCODE_DIV:
        {
            cpu->intfu.result_buffer = cpu->intfu.ps1_value / cpu->intfu.ps2_value;
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;
            cpu->pregs_valid[cpu->intfu.pd] = 1;
            break;
        }
        case OPCODE_AND:
        {
            cpu->intfu.result_buffer = (cpu->intfu.ps1_value) & (cpu->intfu.ps2_value);
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;
            cpu->pregs_valid[cpu->intfu.pd] = 1;
            break;
        }
        case OPCODE_OR:
        {
            cpu->intfu.result_buffer = cpu->intfu.ps1_value | cpu->intfu.ps2_value;
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;
            cpu->pregs_valid[cpu->intfu.pd] = 1;
            break;
        }
        case OPCODE_XOR:
        {
            cpu->intfu.result_buffer = (cpu->intfu.ps1_value) ^ (cpu->intfu.ps2_value);
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;
            cpu->pregs_valid[cpu->intfu.pd] = 1;
            break;
        }
        case OPCODE_ADDL:
        {
            cpu->intfu.result_buffer = (cpu->intfu.ps1_value) + cpu->intfu.imm;
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;
            cpu->pregs_valid[cpu->intfu.pd] = 1;
            break;
        }
        case OPCODE_SUBL:
        {
            cpu->intfu.result_buffer = cpu->intfu.ps1_value - cpu->intfu.imm;
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;
            cpu->pregs_valid[cpu->intfu.pd] = 1;
            break;
        }

        case OPCODE_MOVC:
        {

            cpu->intfu.result_buffer = cpu->intfu.imm;
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;
            cpu->pregs_valid[cpu->intfu.pd] = 1;
            break;
        }

        case OPCODE_HALT:
        {
            cpu->decode.flush = 1;
            cpu->fetch.flush = 1;
            break;
        }
        }

        /* Copy data from intfu latch to memory latch*/
        // cpu->rob = cpu->intfu;

        cpu->intfu.has_insn = FALSE;
        if (ENABLE_DEBUG_MESSAGES && cpu->intfu.opcode != OPCODE_NULL)
        {
            print_stage_content("intfu", &cpu->intfu);
        }
    }
}

/*
 * INT FU Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_mulfu(APEX_CPU *cpu)
{
    if (cpu->mulfu.flush == 1)
    {
        strcpy(cpu->mulfu.opcode_str, " ");
        cpu->mulfu.opcode = 0x0;
        cpu->mulfu.pc = 0000;
    }

    if (cpu->mulfu.has_insn)
    {
        cpu->mulstage++;
        if (cpu->mulstage == 3)
        {
            switch (cpu->mulfu.opcode)
            {
            case OPCODE_MUL:
            {
                cpu->mulfu.result_buffer = cpu->mulfu.ps1_value * cpu->mulfu.ps2_value;
                cpu->renameTableValues[cpu->mulfu.pd] = cpu->mulfu.result_buffer;
                cpu->pregs_valid[cpu->mulfu.pd] = 1;
                break;
            }
            }

            /* Copy data from mulfu latch to memory latch*/
            // cpu->rob = cpu->mulfu;
            cpu->mulfu.has_insn = FALSE;
            cpu->mulstage = 0;
        }

        if (ENABLE_DEBUG_MESSAGES && cpu->mulfu.opcode != OPCODE_NULL)
        {
            print_stage_content("mulfu", &cpu->mulfu);
        }
    }
}

static void
APEX_jbu1(APEX_CPU *cpu)
{
    if (cpu->jbu1.flush == 1)
    {
        strcpy(cpu->jbu1.opcode_str, " ");
        cpu->jbu1.opcode = 0x0;
        cpu->jbu1.pc = 0000;
    }

    if (cpu->jbu1.has_insn)
    {

        switch (cpu->jbu1.opcode)
        {
        case OPCODE_BZ:

        {
            if (cpu->zero_flag == 0)
            {
                cpu->branch_taken = 1;
            }
            else
            {
               // cpu->branch_taken = 0;
            }
            cpu->branchcomplete=1;
            break;
        }

        case OPCODE_BNZ:
        {
            if (cpu->zero_flag == 0)
            {
               // cpu->branch_taken = 0;

            }
            else
            {
                cpu->branch_taken = 1;
            }
            cpu->branchcomplete=1;
            break;
        }

        case OPCODE_JUMP:
        case OPCODE_JAL:
        {
            // cpu->jmpjal[cpu->jbu1.pc] = 1;
            cpu->branch_taken = 1;
            break;
        }
        }

        /* Copy data from jbu1 latch to memory latch*/
        cpu->jbu2 = cpu->jbu1;
        cpu->jbu1.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES && cpu->jbu1.opcode != OPCODE_NULL)
        {
            print_stage_content("jbu1", &cpu->jbu1);
        }
    }
}

static void
APEX_jbu2(APEX_CPU *cpu)
{
    if (cpu->jbu2.flush == 1)
    {
        strcpy(cpu->jbu2.opcode_str, " ");
        cpu->jbu2.opcode = 0x0;
        cpu->jbu2.pc = 0000;
    }

    if (cpu->jbu2.has_insn)
    {

        

        /* Copy data from jbu1 latch to memory latch*/
        cpu->jbu2.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES && cpu->jbu2.opcode != OPCODE_NULL)
        {
            print_stage_content("jbu2", &cpu->jbu2);
        }
    }
}

void validaterob(node *head,APEX_CPU *cpu){
    node* cursor = head;
    while(cursor != NULL)
    {
        
        switch (cursor->data.opcode){
        
        case OPCODE_JUMP:
        case OPCODE_BZ:
        case OPCODE_BNZ:
        case OPCODE_HALT:
        case OPCODE_NOP:
        case OPCODE_NULL:
        case OPCODE_CMP:
        {
            break;
        }
        case OPCODE_STORE:
        {
            cpu->mem_valid[cpu->decode.imm + cpu->renameTableValues[cpu->decode.ps2]] = 1;
            break;
        }
        case OPCODE_STR:{
            cpu->mem_valid[cpu->renameTableValues[cpu->decode.ps1] + cpu->renameTableValues[cpu->decode.ps2]] = 1;
            break;
        }
        
        default:{
        cpu->pregs_valid[cursor->data.pd] = 1;
        }
        }


        cursor = cursor->next;
    }
}


/*
 * rob Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_rob(APEX_CPU *cpu)
{
    if (cpu->rob.flush == 1)
    {
        strcpy(cpu->rob.opcode_str, " ");
        cpu->rob.opcode = 0x0;
        cpu->rob.pc = 0000;
    }

    if (count(robhead) != 0)
    {
        if (robhead->data.opcode == OPCODE_HALT)
        {
            validaterob(robhead,cpu);
            cpu->pregs_valid[cpu->issueq.pd] =1;
            if (ENABLE_DEBUG_MESSAGES && robhead->data.opcode != OPCODE_NULL)
            {
                print_stage_content("ROB ", &robhead->data);
            }
            return TRUE;
        }
    }
    if (cpu->rob.opcode != 0x0 && cpu->rob.has_insn == TRUE)
    {
        robhead = enqueue(robhead, cpu->rob);
    }
    node *cursor = robhead;
    while (cursor != NULL)
    {
        if (ENABLE_DEBUG_MESSAGES && cursor->data.opcode != OPCODE_NULL)
        {
            print_stage_content("ROB ", &cursor->data);
        }
        cursor = cursor->next;
    }

    int dequeued = TRUE;

    if (count(robhead) != 0)
    {
        while (robhead != NULL && dequeued)
        {
            /* Write result to register file based on instruction type */
            dequeued = FALSE;
            switch (robhead->data.opcode)
            {
            case OPCODE_ADD:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_ADDL:
            {
                //printf("\nVALUES : %d,%d\n", renameTableValues[robhead->data.pd],cpu->pregs_valid[robhead->data.pd]);
                if (cpu->pregs_valid[robhead->data.pd])
                {
                    //cpu->zero_flag =cpu->renameTableValues[robhead->data.pd];

                    cpu->regs[robhead->data.rd] = cpu->renameTableValues[robhead->data.pd];
                    cpu->regs_valid[robhead->data.rd] = 1;
                    dequeued = TRUE;
                    phead = enqueueReg(phead,robhead->data.pd);
                    robhead = dequeue(robhead);
                }
                // cpu->regs_valid[cpu->intfu.rd] = 0;
                
                break;
            }

            case OPCODE_SUB:
            case OPCODE_SUBL:
            {
                if (cpu->pregs_valid[robhead->data.pd])
                {
                    cpu->zero_flag = cpu->renameTableValues[robhead->data.pd];

                    cpu->regs[robhead->data.rd] = cpu->renameTableValues[robhead->data.pd];
                    cpu->regs_valid[robhead->data.rd] = 1;
                    dequeued = TRUE;
                    phead = enqueueReg(phead,robhead->data.pd);
                    robhead = dequeue(robhead);
                }
                // cpu->regs_valid[cpu->intfu.rd] = 0;

                break;
            }

            case OPCODE_LDR:
            {
                if (cpu->mreadybit[robhead->data.pc])
                {
                    if (cpu->pregs_valid[robhead->data.pd])
                    {
                        cpu->regs[robhead->data.rd] = cpu->data_memory[cpu->renameTableValues[robhead->data.ps1] + cpu->renameTableValues[robhead->data.ps2]];
                        cpu->regs_valid[robhead->data.rd] = 1;
                        dequeued = TRUE;
                        phead = enqueueReg(phead,robhead->data.pd);
                        robhead = dequeue(robhead);
                    }
                    else
                    {

                        robhead->data.ps1_value = cpu->renameTableValues[robhead->data.ps1];
                        robhead->data.ps2_value = cpu->renameTableValues[robhead->data.ps2];
                        cpu->memory1 = robhead->data;
                        cpu->memory1.has_insn = TRUE;
                    }
                }
                break;
            }

            case OPCODE_LOAD:
            {

                if (cpu->mreadybit[robhead->data.pc])
                {
                    if (cpu->pregs_valid[robhead->data.pd])
                    {
                        cpu->regs[robhead->data.rd] =  cpu->data_memory[cpu->renameTableValues[robhead->data.ps1] + robhead->data.imm];
                        cpu->regs_valid[robhead->data.rd] = 1;
                        dequeued = TRUE;
                        phead = enqueueReg(phead,robhead->data.pd);
                        robhead = dequeue(robhead);
                    }
                    else
                    {

                        robhead->data.ps1_value = cpu->renameTableValues[robhead->data.ps1];
                        cpu->memory1 = robhead->data;
                        cpu->memory1.has_insn = TRUE;
                    }
                }
                break;
            }

            case OPCODE_BZ:
            case OPCODE_BNZ:
            {
                if(cpu->branchcomplete==1){
                if (cpu->branch_taken == 1)
                {
                    // printf("IN BRANCH TAEN");
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = robhead->data.imm + robhead->data.pc;
                    // cpu->decode.has_insn = FALSE;
                    // cpu->fetch.has_insn = TRUE;
                    cpu->intfu.flush = 1;
                    cpu->decode.flush = 1;
                    cpu->branch_taken = 0;
                    dequeued = TRUE;
                    //  robhead = dequeue(robhead);
                    validaterob(robhead,cpu);
                    dispose(robhead);
                    robhead = dequeue(robhead);
                    cpu->rob.flush = 1;
                    dispose(iqhead);
                     iqhead = dequeue(iqhead);
                    cpu->issueq.flush = 1;
                    cpu->intfu.flush = 1;
                    cpu->mulfu.flush = 1;
                    cpu->jbu1.flush=1;
                   // cpu->jbu2.flush=1;
                    cpu->memory1.flush = 1;
                    cpu->memory2.flush = 1;
                    cpu->branchcomplete=0;
                }
                else
                {
                     robhead = dequeue(robhead);
                     dequeued = TRUE;
                     cpu->branchcomplete=0;
                }
                }

                break;
            }
            case OPCODE_JUMP:
            {
                if (cpu->branch_taken == 1)
                {
                    // printf("IN BRANCH TAEN");
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = robhead->data.imm + cpu->renameTableValues[robhead->data.ps1];
                    // cpu->decode.has_insn = FALSE;
                    // cpu->fetch.has_insn = TRUE;
                    cpu->intfu.flush = 1;
                    cpu->decode.flush = 1;
                    cpu->branch_taken = 0;
                    dequeued = TRUE;
                    validaterob(robhead,cpu);
                    //  robhead = dequeue(robhead);
                    dispose(robhead);
                    robhead = dequeue(robhead);

                    cpu->rob.flush = 1;
                    dispose(iqhead);
                     iqhead = dequeue(iqhead);
                    cpu->issueq.flush = 1;
                    cpu->intfu.flush = 1;
                    cpu->mulfu.flush = 1;
                     cpu->jbu1.flush=1;
                   // cpu->jbu2.flush=1;
                    cpu->memory1.flush = 1;
                    cpu->memory2.flush = 1;
                }
                break;
            }

            case OPCODE_JAL:
            {
                if (cpu->branch_taken == 1)
                {
                    // printf("IN BRANCH TAEN");
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = robhead->data.imm + cpu->renameTableValues[robhead->data.ps1];
                    cpu->renameTableValues[robhead->data.pd] = robhead->data.pc + 4;
                    cpu->regs[robhead->data.rd] = cpu->renameTableValues[robhead->data.pd];
                    cpu->regs_valid[robhead->data.rd] = 1;

                    // cpu->decode.has_insn = FALSE;
                    // cpu->fetch.has_insn = TRUE;
                    cpu->intfu.flush = 1;
                    cpu->decode.flush = 1;
                    cpu->branch_taken = 0;
                    dequeued = TRUE;
                    //  robhead = dequeue(robhead);
                    validaterob(robhead,cpu);

                    dispose(robhead);
                    robhead = dequeue(robhead);
                    cpu->rob.flush = 1;
                    dispose(iqhead);
                     iqhead = dequeue(iqhead);
                    cpu->issueq.flush = 1;
                    cpu->jbu1.flush=1;
                    //cpu->jbu2.flush=1;
                    cpu->intfu.flush = 1;
                    cpu->mulfu.flush = 1;
                    cpu->memory1.flush = 1;
                    cpu->memory2.flush = 1;
                }
                break;
            }

            case OPCODE_STR:
            {

                if (cpu->mreadybit[robhead->data.pc])
                {
                   // if (cpu->mem_valid[robhead->data.ps2_value + robhead->data.ps1_value])
                    if(cpu->mem_valid[cpu->renameTableValues[robhead->data.ps1] + cpu->renameTableValues[robhead->data.ps2]])
                    {
                        //cpu->regs[robhead->data.rd] = cpu->renameTableValues[robhead->data.pd];
                        cpu->data_memory[robhead->data.ps1_value + robhead->data.ps2_value] = cpu->renameTableValues[robhead->data.pd];
                        //cpu->regs_valid[robhead->data.rd] = 1;
                        dequeued = TRUE;
                        phead = enqueueReg(phead,robhead->data.pd);
                        robhead = dequeue(robhead);
                        //  printf("VALUE PASSED1");
                    }
                    else
                    {

                        robhead->data.ps1_value = cpu->renameTableValues[robhead->data.ps1];
                        robhead->data.ps2_value = cpu->renameTableValues[robhead->data.ps2];
                        cpu->memory1 = robhead->data;
                        cpu->memory1.has_insn = TRUE;
                        //  printf("VALUE PASSED");
                    }
                }
                break;
            }

            case OPCODE_STORE:
            {

                if (cpu->mreadybit[robhead->data.pc])
                {
                    if (cpu->mem_valid[cpu->renameTableValues[robhead->data.ps2] + robhead->data.imm])
                    {
                        //cpu->regs[robhead->data.rd] = cpu->renameTableValues[robhead->data.pd];
                        cpu->data_memory[robhead->data.ps2_value + robhead->data.imm] = cpu->renameTableValues[robhead->data.ps1];
                        //cpu->regs_valid[robhead->data.rd] = 1;
                        dequeued = TRUE;
                        phead = enqueueReg(phead,robhead->data.pd);
                        robhead = dequeue(robhead);
                        cpu->memory1.has_insn = FALSE;
                        //  printf("VALUE PASSED1");
                    }
                    else
                    {

                        robhead->data.ps1_value = cpu->renameTableValues[robhead->data.ps1];
                        robhead->data.ps2_value = cpu->renameTableValues[robhead->data.ps2];
                        cpu->memory1 = robhead->data;
                        cpu->memory1.has_insn = TRUE;
                        //  printf("VALUE PASSED");
                    }
                }
                break;
            }

            case OPCODE_CMP:
            {
                if (cpu->cmp_completed == 1)
                {
                    cpu->zero_flag = cpu->cmpvalue[robhead->data.pc];
                    dequeued = TRUE;
                    phead = enqueueReg(phead,robhead->data.pd);
                    robhead = dequeue(robhead);
                    cpu->cmp_completed = 0;
                }
                break;
            }

            case OPCODE_MOVC:
            {
                if (cpu->pregs_valid[robhead->data.pd])
                {

                    cpu->regs[robhead->data.rd] = cpu->renameTableValues[robhead->data.pd];
                    cpu->regs_valid[robhead->data.rd] = 1;
                    dequeued = TRUE;
                    phead = enqueueReg(phead,robhead->data.pd);
                    robhead = dequeue(robhead);
                }
                break;
            }
            case OPCODE_HALT:
            {
                //cpu->memory.flush = 1;
                dequeued = FALSE;

                break;
            }
            default:
            {
                robhead = dequeue(robhead);
                dequeued = TRUE;
            }
            }
        }
        cpu->decode.stalled = 0;
        cpu->insn_completed++;
        cpu->rob.has_insn = FALSE;
    }

    /* Default */
    return 0;
}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory1(APEX_CPU *cpu)
{
    if (cpu->memory1.has_insn)
    {

        switch (cpu->memory1.opcode)
        {

        case OPCODE_LOAD:
        case OPCODE_LDR:
        {
            cpu->pregs_valid[cpu->memory1.pd] = 1;
            break;
        }

        case OPCODE_STORE:
        {
            // cpu->data_memory[cpu->memory1.result_buffer] = cpu->memory1.rs1_value;
            // cpu->mem_valid[cpu->memory1.result_buffer] = 1;
            // cpu->decode.stalled = 0;
            cpu->mem_valid[cpu->memory1.ps2_value + cpu->memory1.imm] = 1;
            break;
        }
        case OPCODE_STR:
        {
            // cpu->data_memory[cpu->memory1.result_buffer] = cpu->memory1.rd_value;
            // cpu->mem_valid[cpu->memory1.result_buffer] = 1;
            // cpu->decode.stalled = 0;
            cpu->mem_valid[cpu->memory1.ps2_value + cpu->memory1.ps1_value] = 1;
            break;
        }

        case OPCODE_HALT:
        {
            cpu->intfu.flush = 1;
            cpu->fetch.flush = 1;
            break;
        }
        }

        /* Copy data from memory1 latch to rob latch*/
        cpu->memory2 = cpu->memory1;

        cpu->memory1.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES && cpu->memory1.opcode != OPCODE_NULL)
        {
            print_stage_content("Memory1", &cpu->memory1);
        }
    }
}
/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory2(APEX_CPU *cpu)
{
    if (cpu->memory2.has_insn)
    {

        switch (cpu->memory2.opcode)
        {

        case OPCODE_LDR:
        {

            cpu->memory2.result_buffer = cpu->memory2.ps1_value + cpu->memory2.ps2_value;
            cpu->renameTableValues[cpu->memory2.pd] = cpu->data_memory[cpu->memory2.result_buffer];
            // cpu->regs[cpu->memory2.rd] = cpu->renameTableValues[cpu->memory2.pd];
            // cpu->regs_valid[cpu->memory2.rd] = 1;
            // cpu->pregs_valid[cpu->memory2.pd] = 1;
            break;
        }

        case OPCODE_LOAD:
        {
            cpu->memory2.result_buffer = cpu->memory2.ps1_value + cpu->memory2.imm;
            cpu->renameTableValues[cpu->memory2.pd] = cpu->data_memory[cpu->memory2.result_buffer];
            ;
            break;
        }

        case OPCODE_STORE:
        case OPCODE_STR:
        {
            break;
        }
        case OPCODE_BZ:
        case OPCODE_BNZ:
        {
            if (cpu->branch_taken == 1)
            {

                /* Calculate new PC, and send it to fetch unit */
                cpu->pc = cpu->memory2.pc + cpu->memory2.imm;
                cpu->decode.has_insn = FALSE;
                cpu->fetch.has_insn = TRUE;
                cpu->intfu.flush = 1;
                cpu->decode.flush = 1;
                cpu->branch_taken = 0;
            }
            break;
        }

        case OPCODE_HALT:
        {
            cpu->intfu.flush = 1;
            cpu->fetch.flush = 1;
            break;
        }
        }

        /* Copy data from memory2 latch to rob latch*/
        //cpu->rob = cpu->memory2;
        cpu->memory2.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES && cpu->memory2.opcode != OPCODE_NULL)
        {
            print_stage_content("Memory2", &cpu->memory2);
        }
    }
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{

    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->renameTableValues, 0, sizeof(int) * (PREGS_FILE_SIZE+1));

    iqhead = NULL;
    robhead = NULL;
    phead = NULL;
    rfprf = NULL;

    tmp = NULL;
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    memset(cpu->mreadybit, 0, sizeof(int) * 60000);
    memset(cpu->cmpvalue, 0, sizeof(int) * 60000);
    for (i = 0; i < 4096; i++)
    {
        cpu->mem_valid[i] = 1;
    }
    cpu->zero_flag = -9999;
    cpu->branch_taken = 0;
    cpu->cmp_completed = 0;
    cpu->mulstage = 0;
    cpu->branchcomplete = 0;
    // cpu->data_memory[124031] = 1;
    for (i = 0; i < 16; i++)
    {
        cpu->regs_valid[i] = 1;
    }
    for (i = 0; i < 48; i++)
    {                  
        cpu->pregs_valid[i] = 1;
        phead = enqueueReg(phead, i);
    }
    

    cpu->single_step = ENABLE_SINGLE_STEP;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;

    return cpu;
}

void printFile(APEX_CPU *cpu)
{


    printf("\n-----------------REGISTER FILE------------------------------------------------------- \n");
    printf("|Ar Register|Phy. Register| Value | VALID bit\n");
    for (int i = 0; i < 16; i++)
    {
        printf("|R[%d]\t|\tP[%d]\t|\t=%d\t|\t%d\n",i,searchprftop1(rfprf, i),cpu->renameTableValues[searchprftop(rfprf, i)],cpu->pregs_valid[searchprftop1(rfprf, i)] );
    }
    printf("\n-----------------REGISTER FILE------------------------------------------------------- \n");






    printf("\n-----------------DATA MEMORY-------------- \n");
    for (int i = 0; i < 4096; i++)
    {
        if (cpu->data_memory[i] != 0)
        {

            printf("| MEM[%d] | Value=%d | \n", i, cpu->data_memory[i]);
        }
    }
    printf("-----------------DATA MEMORY-------------- \n");
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void APEX_cpu_run(APEX_CPU *cpu, int x, int y)
{

    switch (x)
    {
    case 1:
    {
        APEX_run_at_choice(cpu, 0);
        printFile(cpu);
        break;
    }
    case 2:
    {
        int p = 0;
        while (p < y)
        {
            APEX_run_at_choice(cpu, 1);
            printFile(cpu);
            p++;
        }
        
        break;
    }
    case 3:
    {
        int breaker = 0;
        while (breaker != 1)
        {
            breaker = APEX_run_at_choice(cpu, 0);
            printFile(cpu);
        }
        
        break;
    }
    case 4:
    {
        int breaker = 0;
        while (breaker != 1)
        {
            breaker = APEX_run_at_choice(cpu, 1);
            printFile(cpu);
        }
        
        break;
    }
    case 5:
    {
        printf("MEM[%d] | Value=%d", y, cpu->data_memory[y]);
        break;
    }
    default:
    {
        return;
    }
    }
}

int APEX_run_at_choice(APEX_CPU *cpu, int z)
{
    int breaker = 0;
    char user_prompt_val;
    if (ENABLE_DEBUG_MESSAGES)
    {
        printf("--------------------------------------------\n");
        printf("Clock Cycle #: %d\n", cpu->clock + 1);
        printf("--------------------------------------------\n");
    }

    APEX_mulfu(cpu);
    APEX_intfu(cpu);

    if (APEX_rob(cpu))
    {
        /* Halt in rob stage */
        printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock + 1, cpu->insn_completed);
        return 1;
    }
    APEX_jbu2(cpu);
    APEX_jbu1(cpu);
    APEX_memory2(cpu);
    APEX_memory1(cpu);
    APEX_issueq(cpu);
    APEX_decode(cpu);
    APEX_fetch(cpu);

    print_reg_file(cpu);
    

    if (cpu->single_step && z == 0)
    {
        printf("Press any key to advance CPU Clock or <q> to quit:\n");
        scanf("%c", &user_prompt_val);

        if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
        {
            printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            breaker = 1;
        }
    }

    cpu->clock++;
    return breaker;
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);

}