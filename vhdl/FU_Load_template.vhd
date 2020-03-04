library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.math_real."ceil";
use IEEE.math_real."log2";


entity FU_LOAD is
    generic (
                INSTRUCTIONS : natural;
                TOTAL_EXE_CYCLES : natural;
                BITWIDTH : natural; -- 31 -> meaning 32
                RF_DEPTH: natural; --15 -> meaning 16
                OUTPUT_PORTS : natural
            );
    port(
        i_clock : in std_logic;
        i_FU : in std_logic_vector(BITWIDTH downto 0);
        o_FU : out std_logic_vector(((BITWIDTH +1)*OUTPUT_PORTS)-1 downto 0);
        r_COMPUTING : in std_logic;
        r_LOAD_INST: in std_logic := '0';
        r_LOAD_NEXT_INST: in std_logic ;
        r_INPUT_INST : in std_logic_vector(
                natural(ceil(log2(real(TOTAL_EXE_CYCLES+1))))+ -- bit required to ID Clock
                1 + -- Register Write enable bit 
                natural(ceil(log2(real(RF_DEPTH)))) * 2  -- RF select bits (Read and Write addrs)
                -1
                downto 0);
        r_RESET : std_logic
        );
end FU_LOAD;

architecture rtl of FU_LOAD is
    constant RFSelectBits : natural := natural(ceil(log2(real(RF_DEPTH))));
    -- Write Enable + Waddress + Raddress
    constant INSTRUCTION_SIZE :natural := 1 + RFSelectBits * 2;
    
    -- SIGNALS FOR INSTRUCTION MEMORY
    signal w_OUTPUT_INST : std_logic_vector(INSTRUCTION_SIZE-1 downto 0); -- INSTRUCTION_SIZE
    signal w_VALID_INST : std_logic := '0';

    -- SIGNAL FOR FU OUTPUT
    signal opOUT: std_logic_vector(BITWIDTH downto 0) := (OTHERS => '0');
    signal READ_ADDR: std_logic_vector(3 downto 0) := (OTHERS => '0');
    signal WRITE_ADDR: std_logic_vector(3 downto 0) := (OTHERS => '0');
    signal WRITE_EN : std_logic;
 
    -- SIGNALS FOR REGISTER FILE
    signal w_OUTPUT_A : std_logic_vector(BITWIDTH downto 0);
    signal r_INPUT_1 : std_logic_vector(BITWIDTH downto 0);

    component instruction_memory is
    generic (MAX_INSTRUCTION : natural;
             INSTRUCTION_SIZE : natural;
             TOT_NUM_CYCLES : natural);
        port
        (
        clock : in std_logic;
        computing : in std_logic;
        load_inst : in std_logic;
        load_next_inst : in std_logic;
        input_inst : in std_logic_vector(natural(ceil(log2(real(TOTAL_EXE_CYCLES+1))))+ INSTRUCTION_SIZE-1 downto 0);
        output_inst : out std_logic_vector(INSTRUCTION_SIZE-1 downto 0); -- INSTRUCTION_SIZE
        valid_inst : out std_logic :='0';
        reset : in std_logic
        );
    end component instruction_memory;
    component register_file_singleIO is
        generic  
        (
            BITWIDTH : natural; --31
            DEPTH: natural -- 15
        );
        port
        (
            outA: out std_logic_vector(BITWIDTH downto 0);
            input1: in std_logic_vector(BITWIDTH downto 0);
            writeEnable1: in std_logic;

            --the select signal needs to be log2(registerfilesize)
            reg1Sel_read : in std_logic_vector(natural(ceil(log2(real(DEPTH+1))))-1 downto 0);
            reg1Sel_write : in std_logic_vector(natural(ceil(log2(real(DEPTH+1))))-1 downto 0);
            clock : in std_logic;
            reset : in std_logic
        );
    end component register_file_singleIO;
begin
    --Instantiate the Unit Under Test (UUT)
    IM : instruction_memory
        generic map (
            MAX_INSTRUCTION => INSTRUCTIONS,
            INSTRUCTION_SIZE => INSTRUCTION_SIZE,
            TOT_NUM_CYCLES => TOTAL_EXE_CYCLES
        )  
        port map (
        clock => i_clock,
        computing => r_COMPUTING ,
        load_inst => r_LOAD_INST ,
        load_next_inst => r_LOAD_NEXT_INST ,
        input_inst => r_INPUT_INST ,
        output_inst => w_OUTPUT_INST ,
        valid_inst => w_VALID_INST ,
        reset => r_RESET
                 );

    RF : register_file_singleIO 
        generic map (
            BITWIDTH => BITWIDTH,
            DEPTH => RF_DEPTH
                    )
        port map (
        outA => w_OUTPUT_A,
        input1 => i_FU,
        writeEnable1 => w_OUTPUT_INST(INSTRUCTION_SIZE-1),

        reg1Sel_read  => w_OUTPUT_INST(INSTRUCTION_SIZE -2 
                            downto INSTRUCTION_SIZE- 1 - RFSelectBits),
        reg1Sel_write  => w_OUTPUT_INST(INSTRUCTION_SIZE - RFSelectBits -2  
        downto 0 ),
        clock => i_clock,
        reset  => r_RESET
                 );
        READ_ADDR <= w_OUTPUT_INST(INSTRUCTION_SIZE -2 
                            downto INSTRUCTION_SIZE- 1 - RFSelectBits);
        WRITE_ADDR <= w_OUTPUT_INST(INSTRUCTION_SIZE - RFSelectBits -2  
                            downto 0 );
        WRITE_EN <= w_OUTPUT_INST(INSTRUCTION_SIZE-1);
    genOutputPorts: for i in 0 to (OUTPUT_PORTS-1) generate
            o_FU(((i+1)*(BITWIDTH+1)-1) downto (i*(BITWIDTH+1))) <= w_OUTPUT_A;
    end generate genOutputPorts;
    --fuProc : process(i_clock ) is
    --begin 
    --    if r_COMPUTING = '1' then
    --        if w_VALID_INST = '1' then
    --            for i in 0 to (OUTPUT_PORTS -1 ) loop
    --                o_FU(((i+1)*(BITWIDTH+1)-1) downto (i*(BITWIDTH+1))) <= w_OUTPUT_A;
    --            end loop;
    --        else 
    --            o_FU <= (OTHERS => '0');
    --        end if;
    --    end if;
    --end process;
end rtl;
