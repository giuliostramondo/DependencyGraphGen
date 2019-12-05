library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.math_real."ceil";
use IEEE.math_real."log2";


-- extended from https://stackoverflow.com/questions/19942067/writing-a-register-file-in-vhdl
entity register_file_singleIO is
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
end register_file_singleIO;

architecture behavioral of register_file_singleIO is
    type registerFile is array(0 to DEPTH) of std_logic_vector(BITWIDTH downto 0);
    signal registers : registerFile;
begin
    regFile : process(clock) is
    begin
        if rising_edge(clock) then
            outA <= registers(to_integer(unsigned(reg1Sel_read)));
            if writeEnable1 = '1' then
                registers(to_integer(unsigned(reg1Sel_write))) <= input1;
                if reg1Sel_read = reg1Sel_write then  -- Bypass for read A
                  outA <= input1;
                end if;
            end if;
            if reset = '1' then
                registers <= (OTHERS => (OTHERS => '0'));
            end if;
        end if;
    end process;
end behavioral;
