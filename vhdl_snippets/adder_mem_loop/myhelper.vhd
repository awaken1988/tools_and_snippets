package myhelper is
    constant SOME_FLAG : bit_vector := "11111111";
    function addr_ld(m:positive) return natural;
end package myhelper;

package body myhelper is
    function addr_ld(m:positive) return natural is
    begin
       for n in 0 to integer'high loop
          if (2**n >= m) then
             return n;
          end if;
       end loop;
    end function addr_ld;
end package body;