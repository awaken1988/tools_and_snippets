library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity simple_testbench is
    Port(
        a: in STD_LOGIC;
        b: in STD_LOGIC;
        c: in STD_LOGIC;
        j: out STD_LOGIC;
        K: out STD_LOGIC
    );
end simple_testbench;

architecture behv of simple_testbench is
    signal abc : std_logic_vector(2 downto 0);
    signal hidden_abc: std_logic_vector(2 downto 0);
begin
    abc <=  a & b & c;
    hidden_abc <= a & b & c;
    
    process(abc) 
      
    begin
        j <= abc(2) and abc(1) and abc(0);
        k <= abc(2) or abc(1) or abc(0);
    end process;
    
   
end behv;