library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;



-- extended from https://stackoverflow.com/questions/19942067/writing-a-register-file-in-vhdl
entity register_file_v2 is
    port
    (
    outA: out std_logic_vector(31 downto 0);
    outB: out std_logic_vector(31 downto 0);
    input1: in std_logic_vector(31 downto 0);
    input2: in std_logic_vector(31 downto 0);
    writeEnable1: in std_logic;
    writeEnable2: in std_logic;

    --the select signal needs to be log2(registerfilesize)
    reg1Sel_read : in std_logic_vector(3 downto 0);
    reg2Sel_read : in std_logic_vector(3 downto 0);
    reg1Sel_write : in std_logic_vector(3 downto 0);
    reg2Sel_write : in std_logic_vector(3 downto 0);
    clock : in std_logic;
    reset : in std_logic
    );
end register_file_v2;

architecture behavioral of register_file_v2 is
    type registerFile is array(0 to 7) of std_logic_vector(31 downto 0);
    signal registers_1 : registerFile;
    signal registers_2 : registerFile;
begin
    regFile : process(clock) is
    begin
        if rising_edge(clock) then
            if reg1Sel_read(0) = '1' then
                outA <= registers_2(to_integer(unsigned(reg1Sel_read(3 downto 1))));
            else 
                outA <= registers_1(to_integer(unsigned(reg1Sel_read(3 downto 1))));
            end if;
            if reg2Sel_read(0) = '1' then
                outB <= registers_2(to_integer(unsigned(reg2Sel_read(3 downto 1))));
            else
                outB <= registers_1(to_integer(unsigned(reg2Sel_read(3 downto 1))));
            end if;
            if writeEnable1 = '1' then
                if reg1Sel_write(0) = '1' then
                    registers_2(to_integer(unsigned(reg1Sel_write(3 downto 1)))) <= input1;
                else
                    registers_1(to_integer(unsigned(reg1Sel_write(3 downto 1)))) <= input1;
                end if;
            end if;
            if writeEnable2 = '1' then
                if reg2Sel_write(0) = '1' then
                    registers_2(to_integer(unsigned(reg2Sel_write(3 downto 1)))) <= input2;
                else
                    registers_1(to_integer(unsigned(reg2Sel_write(3 downto 1)))) <= input2;
                end if;
            end if;
            if reset = '1' then
                registers_1 <= (OTHERS => (OTHERS => '0'));
                registers_2 <= (OTHERS => (OTHERS => '0'));
            end if;
        end if;
    end process;
end behavioral;
