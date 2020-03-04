library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.math_real."ceil";
use IEEE.math_real."log2";


-- extended from https://stackoverflow.com/questions/19942067/writing-a-register-file-in-vhdl
entity FU_register_file_paramIn_2Out is
    generic  
    (
        BITWIDTH : natural; --31
        DEPTH: natural; -- 15
        WRITE_LANES: natural -- 3
    );

    port
    (
    outA: out std_logic_vector(BITWIDTH downto 0);
    outB: out std_logic_vector(BITWIDTH downto 0);
    input1: in std_logic_vector(((BITWIDTH+1)*WRITE_LANES)-1 downto 0);
    writeEnable1: in std_logic_vector(WRITE_LANES-1 downto 0);

    --the select signal needs to be log2(registerfilesize)
    reg1Sel_read : in std_logic_vector(natural(ceil(log2(real(DEPTH+1))))-1 downto 0);
    reg2Sel_read : in std_logic_vector(natural(ceil(log2(real(DEPTH+1))))-1 downto 0);
    reg1Sel_write : in std_logic_vector((natural(ceil(log2(real(DEPTH+1))))*WRITE_LANES) -1 downto 0);
    clock : in std_logic;
    reset : in std_logic
    );
end FU_register_file_paramIn_2Out;

architecture behavioral of FU_register_file_paramIn_2Out is
    type registerFile is array(0 to DEPTH) of std_logic_vector(BITWIDTH downto 0);
    signal registers : registerFile;

    constant RF_ADDR_SIZE : natural := natural(ceil(log2(real(DEPTH+1))));
begin
    regFile : process(clock) is
    variable RF_ADDR_MSB : natural range 0 to (RF_ADDR_SIZE*WRITE_LANES)-1;
    variable RF_ADDR_LSB : natural range 0 to (RF_ADDR_SIZE*WRITE_LANES)-1;
    variable INPUT_MSB : natural range 0 to ((BITWIDTH+1)*WRITE_LANES)-1;
    variable INPUT_LSB : natural range 0 to ((BITWIDTH+1)*WRITE_LANES)-1;
    begin
        if rising_edge(clock) then
            outA <= registers(to_integer(unsigned(reg1Sel_read)));
            outB <= registers(to_integer(unsigned(reg2Sel_read)));
            --if writeEnable1 = '1' then
            --    registers(to_integer(unsigned(reg1Sel_write))) <= input1;
            --end if;
            --if writeEnable2 = '1' then
            --    registers(to_integer(unsigned(reg2Sel_write))) <= input2;
            --end if;
            for ii in 0 to WRITE_LANES-1 loop
                if writeEnable1(WRITE_LANES-ii-1) = '1' then
                    -- WRITE THE TEST BENCH!!
                    RF_ADDR_MSB := (RF_ADDR_SIZE*(WRITE_LANES-ii)-1);
                    RF_ADDR_LSB := (RF_ADDR_SIZE*(WRITE_LANES-ii-1));
                    INPUT_MSB :=   ((BITWIDTH+1)*(WRITE_LANES-ii)-1);
                    INPUT_LSB :=   ((BITWIDTH+1)*(WRITE_LANES-ii-1));
                    report "Selected Input  = " & integer'image(to_integer(unsigned(input1(INPUT_MSB downto INPUT_LSB)))) & " store at register position:"& integer'image(to_integer(unsigned(reg1Sel_write(RF_ADDR_MSB downto RF_ADDR_LSB)))) & " write enable: " & integer'image(to_integer(unsigned(writeEnable1))); 
                    registers(to_integer(unsigned(reg1Sel_write(RF_ADDR_MSB downto RF_ADDR_LSB)))) <=  input1(INPUT_MSB downto INPUT_LSB);
                end if;
            end loop;
            if reset = '1' then
                registers <= (OTHERS => (OTHERS => '0'));
            end if;
        end if;
    end process;
end behavioral;
