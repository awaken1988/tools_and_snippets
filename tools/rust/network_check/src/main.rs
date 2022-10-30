use std::{net::{IpAddr, Ipv4Addr, Ipv6Addr}, time::Duration, thread, time::SystemTime, iter, fs, fs::File, io::Write};
use chrono::{Datelike, Timelike, Utc, Local};
use dns_lookup::{lookup_host, lookup_addr};
use ping::ping;
use itertools::Itertools;

struct PingChecker
{
    name: String,
    idx:  usize,
    addr: Vec<IpAddr>,
}

impl PingChecker {
    fn new(addresses: Vec<IpAddr>, name: &str) -> PingChecker {
        PingChecker {
            name: name.to_string(),
            idx:  0,
            addr: addresses
        }
    }

    fn next_id(&mut self) -> usize {
        self.idx+=1;
        if self.idx >= self.addr.len() {
            self.idx=0;
        }
            
        return self.idx
    } 

    fn check(&mut self) -> Option<IpAddr>{
        for i_round in 0..self.addr.len() {
            let idx = self.next_id();
            let ip_addr = self.addr[idx];

            let ping_result = ping(
                ip_addr,
                Some(Duration::from_secs(2)),
                Option::None,
                Option::None,
                Option::None,
                Option::None);

            if let Ok(()) = ping_result {
                return Some(ip_addr);
            }
        }

        return None;
    }
}

fn main() {

    let hostnames = vec![
        "www.google.de".to_string(),
        "www.heise.de".to_string(),
        "www.chip.de".to_string(),
        "8.8.8.8".to_string(),
        "8.8.4.4".to_string(),
        "9.9.9.9".to_string(),
        "1.1.1.1".to_string(),
        "2001:4860:4860::8888".to_string(),
        "2001:4860:4860::8844".to_string(),
    ];

    let addresses = hostnames.iter()
        .map(|x|{ lookup_host(x) })
        .map_while(|x| if let Ok(y) = x {Some(y)} else { Option::None }  )
        .flatten()
        .collect::<Vec<IpAddr>>();

    let mut checker = {
        let v4 = addresses.iter().cloned()
            .filter(|x| if let IpAddr::V4(y) = x {true} else {false} )
            .collect::<Vec<IpAddr>>();

        let v6 = addresses.iter().cloned()
         .filter(|x| if let IpAddr::V6(y) = x {true} else {false} )
            .collect::<Vec<_>>();

        vec![PingChecker::new(v4, "ip4"), PingChecker::new(v6, "ip6")]
    };

    let mut outfile = File::create("out.txt").expect("cannot open logfile");

    loop {
        //let now = SystemTime::now();
        //let sys_time = SystemTime::now().duration_since(SystemTime::UNIX_EPOCH).expect("cannot get unixtime");

        let dt = Local::now();

        let mut output_result = String::new();
        let mut output_detail: String = String::new();

        for i in checker.iter_mut() {
            let mut result =i.check();

            let result_str       =  if let Some(x) = result {"y"} else {"n"};
            let address_string =  if let Some(x) = result {x.to_string()} else {".".to_string()};

            output_result = format!("{} {}", output_result, result_str);
            output_detail = format!("{} {}", output_detail, address_string);
        }

        let output  = format!("{} {} {}", dt.format("%Y-%m-%d__%H:%M:%S"), output_result, output_detail);


        println!("{}", output);
        outfile.write(format!("{}\n",output).as_bytes());

        thread::sleep(Duration::from_secs(4));
    }
}
