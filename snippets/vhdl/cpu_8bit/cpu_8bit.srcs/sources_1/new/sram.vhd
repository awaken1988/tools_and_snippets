----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 04/13/2020 04:20:02 PM
-- Design Name: 
-- Module Name: sram - Behavioral
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
use IEEE.NUMERIC_STD.ALL;
use work.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity sram is
    Port ( clk : in STD_LOGIC;
           write_en : in STD_LOGIC;
           address  : in STD_LOGIC_VECTOR(7 downto 0);
           datain   : in STD_LOGIC_VECTOR(7 downto 0);
           dataout  : out STD_LOGIC_VECTOR(7 downto 0) );
end sram;

architecture Behavioral of sram is
    signal content: project_types.ram_t;
    signal read_addr: std_logic_vector(7 downto 0);
begin

    process(clk, write_en)
    begin
        if rising_edge(clk) then
            if write_en = '1' then
                content(to_integer(unsigned(address))) <= datain;
            end if;
            read_addr <= address;
        end if;  
    end process;

    dataout <= content(to_integer(unsigned(read_addr)));

end Behavioral;
