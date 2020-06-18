library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

package project_types is
    type ram_t is array(255 downto 0) of STD_LOGIC_VECTOR(7 downto 0 );
end package project_types;