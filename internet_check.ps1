Param (
    [string]$router  = "192.168.0.1",     
    [string]$lanhost = "192.168.0.100"                
)

$ping_hosts = @(
    @{
            name = "wan";
            hosts = @("www.google.de", "www.heise.de", "www.golem.de", "www.wikipedia.org", "208.67.222.222" , "1.1.1.1", "1.0.0.1" ,"8.8.8.8", "8.8.4.4");
            idx   = 0
    };
    @{
            name  = "router"
            hosts = @($router);
            idx   = 0
    }
    @{
            name  = "lanhost"
            hosts = @($lanhost);
            idx   = 0
    }
);


function PingHost() {
    param (
        $ping_state
    )

    $i_max = $ping_state.hosts.Count
    $i     = 0
    while( $i -lt $i_max) {
        if( $ping_state.idx -ge $i_max ) {
            $ping_state.idx = 0
        }

        $host_name = $ping_state.hosts[$ping_state.idx]
        if( Test-Connection -Quiet -Count 1 $host_name ) {
            $ping_state.idx++; 
            return @{ok=$true; host=$host_name; name=$ping_state.name}
        }
        $ping_state.idx++;
        return @{ok=$false; host=$host_name; name=$ping_state.name}
    }
}

while($true) {
    Start-Sleep -Seconds 1

    $results = @();

    foreach($iPingState in $ping_hosts) {
        $curr = PingHost -ping_state $iPingState
        $results += $curr
    }

    Write-Host -NoNewline (Get-Date -Format "yyyy-MM-dd HH:mm:ss")
    foreach($iResult in $results) {
        #alternative output
        #$str = "{0,24} " -f $iResult.host
        #if( $iResult.ok ) {
        #    Write-Host -NoNewline $str "OK    |"
        #}
        #else {
        #    Write-Host -ForegroundColor Red -NoNewline $str "FAIL  |" 
        #}

        $str = "        {0}" -f $iResult.name
        if( $iResult.ok ) {
            Write-Host -NoNewline $str "OK    |"
        }
        else {
            Write-Host -ForegroundColor Red -NoNewline $str "FAIL  |" 
        }
        
    }

    Write-Host "..."
}
