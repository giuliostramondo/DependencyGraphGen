library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.math_real."ceil";
use IEEE.math_real."log2";
    


entity instruction_memory is
    generic (MAX_INSTRUCTION : natural);
    port
    (
        clock : in std_logic;
        computing : in std_logic;
        load_inst : in std_logic;
        load_next_inst : in std_logic;
        -- len of input inst
        -- 2 bits for MuxA
        -- 2 bits for MuxB
        -- 2 bits for Demux after OP
        -- 2 bits for Demux Output ports (if we output on all ports not required)
        -- 4 bits for Reg1SelectWrite
        -- 4 bits for Reg2SelectWrite
        -- 4 bits for Reg1SelectRead
        -- 4 bits for Reg2SelectRead
        -- 1 bit for RegInput1WriteEn
        -- 1 bit for RegInput2WriteEn
        -- 6 bits for inputCrossbarSelect
        -- 8 bits for clock (256 TOT_NUM_CYCLE) log_2(TOT_NUM_CYCLE) to identify clock
        ---------------------------------------
        -- 40 bits total to store
        input_inst : in std_logic_vector(39 downto 0);
        -- The actual inst size is 40 - 8 (bit for clock) 32 bits
        output_inst : out std_logic_vector(31 downto 0);
        valid_inst : out std_logic :='0';
        reset : in std_logic
    );
end instruction_memory;

architecture behavioral of instruction_memory is
    -- for now 8 instruction
    --constant MAX_INSTRUCTION : natural := 8;
    signal INST_POINTER_LOAD : std_logic_vector(natural(ceil(log2(real(MAX_INSTRUCTION+1))))-1 downto 0) := (OTHERS=> '0');
    signal INST_POINTER_COMPUTE : std_logic_vector(natural(ceil(log2(real(MAX_INSTRUCTION+1))))-1 downto 0) := (OTHERS =>'0');
    --signal INST_POINTER_LOAD : std_logic_vector(3-1 downto 0) := "000";
    --signal INST_POINTER_COMPUTE : std_logic_vector(3-1 downto 0) := "000";
    type instMemory is array(0 to MAX_INSTRUCTION) of std_logic_vector(39 downto 0);
    signal imem : instMemory;
    signal c_CLOCK : natural range 0 to 256 := 0;
begin
    clockCount : process(clock) is
    begin 
        if rising_edge(clock) then
            if computing = '1' then
                c_CLOCK <= c_CLOCK +1;
            end if;
        end if;
    end process clockCount;

    instMem : process(clock) is
        variable next_instruction_issue : natural range 0 to 256;
        variable current_instruction : std_logic_vector(39 downto 0);
    begin
        if rising_edge(clock) then
            if load_inst = '1' then
                imem(to_integer(unsigned(INST_POINTER_LOAD))) <= input_inst;
            else
                INST_POINTER_LOAD <="000";
            end if;
            if load_next_inst = '1' then
                INST_POINTER_LOAD <= std_logic_vector(unsigned(INST_POINTER_LOAD) + 1);
            end if;
            if reset = '1' then
                imem <= (OTHERS => (OTHERS => '0'));
            end if;
            if computing = '1' then 
               current_instruction := imem(to_integer(unsigned(INST_POINTER_COMPUTE)));
               next_instruction_issue := to_integer(unsigned(current_instruction(39 downto 32)));
               if next_instruction_issue = c_CLOCK +1  then
                   valid_inst <= '1';
                   output_inst <= current_instruction(31 downto 0);
                   INST_POINTER_COMPUTE <= std_logic_vector(unsigned(INST_POINTER_COMPUTE) + 1);
                else 
                   valid_inst <= '0';
                   output_inst <= (OTHERS => '0');
               end if; 
            else
                INST_POINTER_COMPUTE <= "000";
            end if;
        end if;
    end process;
end behavioral;
