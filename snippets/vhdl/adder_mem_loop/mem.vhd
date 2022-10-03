-- list of literature:
--   http://www.ics.uci.edu/~jmoorkan/vhdlref/generics.html
--   http://www.lothar-miller.de/s9y/archives/72-Breite-eines-Vektors-berechnen-log2.html
--   https://www.csee.umbc.edu/portal/help/VHDL/types.html

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use work.myhelper.all;



entity mem is
    generic (  DATAWIDTH: integer;
               CELLCOUNT: integer);
    port (  I_CLK: std_logic;
            I_WE: std_logic;
            I_ADDR: std_logic_vector(addr_ld(CELLCOUNT)-1 downto 0);
            I_DATA: std_logic_vector(DATAWIDTH-1 downto 0);
            O_DATA: out std_logic_vector(DATAWIDTH-1 downto 0)
         );          
end mem;

architecture Behavioural of mem is
    type store_t is array (CELLCOUNT-1 downto 0) of std_logic_vector(DATAWIDTH-1 downto 0);
    signal content: store_t;
begin
    process(I_CLK)
    begin
        if rising_edge(I_CLK) then
            if I_WE = '1' then
                content(to_integer(unsigned(I_ADDR(addr_ld(CELLCOUNT)-1 downto 0)))) <= I_DATA;
            else 
                O_DATA <= content(to_integer(unsigned(I_ADDR(DATAWIDTH-1 downto 0))));
            end if;
        end if;
    end process;
end Behavioural;

