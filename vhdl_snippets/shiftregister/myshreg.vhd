----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 28.03.2018 15:46:04
-- Design Name: 
-- Module Name: my_shreg - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity myshreg is
    Port (  input:      in std_logic;
            clock:      in std_logic;
            shift:      in std_logic;
            output:     out std_logic;
            dbg:        out std_logic_vector(3 downto 0));
end myshreg;

architecture myshreg_behv of myshreg is
    signal reg: std_logic_vector(3 downto 0) := "0000";
begin
    process(clock)
    begin
        if( rising_edge(clock) ) then
            if( shift = '1' ) then
                reg <= reg(2 downto 0) & input;
            end if;
        end if; 
    end process;

  output <= reg(3);
  dbg <= reg;
end myshreg_behv;
