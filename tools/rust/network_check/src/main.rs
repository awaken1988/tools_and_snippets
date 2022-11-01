use std::{net::{IpAddr, Ipv4Addr, Ipv6Addr}, time::Duration, thread, time::SystemTime, iter, fs, fs::File, io::Write};
use chrono::{Datelike, Timelike, Utc, Local};
use dns_lookup::{lookup_host, lookup_addr};
use ping::ping;
use itertools::Itertools;
use default_net;
use clap::{Parser, Command, Arg};

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
        let max_rounds = std::cmp::min(6, self.addr.len());

        for i_round in 0..max_rounds {
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

fn remote_checker() -> Vec<PingChecker> {
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
        "208.67.222.222".to_string(),
        "208.67.220.220".to_string(),
        "1.1.1.1".to_string(),
        "1.0.0.1".to_string(),
    ];

    let addresses = hostnames.iter()
        .map(|x|{ lookup_host(x) })
        .map_while(|x| if let Ok(y) = x {Some(y)} else { Option::None }  )
        .flatten()
        .collect::<Vec<IpAddr>>();
    let v4 = addresses.iter().cloned()
        .filter(|x| if let IpAddr::V4(y) = x {true} else {false} )
        .collect::<Vec<IpAddr>>();

    let v6 = addresses.iter().cloned()
     .filter(|x| if let IpAddr::V6(y) = x {true} else {false} )
        .collect::<Vec<_>>();

    vec![PingChecker::new(v4, "INTERNET4"), PingChecker::new(v6, "INTERNET6")]
}

fn cli() -> Command {
    Command::new("network_check")
        .about("Test internet connection periodically")
        .arg(Arg::new("out").long("out"))
        .arg(Arg::new("delay").long("delay").default_value("10"))
}

fn main() {
    let matches = cli().get_matches();
    let mut checker = remote_checker();

    //Try to get Gateway
    if let Ok(x) = default_net::get_default_gateway() {
        checker.push(PingChecker::new(vec![x.ip_addr], "ROUTER"));
    };

    //Output file
    let mut out: Option<File>= {
        if let Some(outpath) = matches.get_one::<String>("out") {
            Some(File::create(outpath).expect("cannot open logfile"))
        }
        else {
            Option::None
        }
    };  

    loop {
        let dt = Local::now();

        let mut output_result = String::new();
        let mut output_detail: String = String::new();

        for i in checker.iter_mut() {
            let mut result =i.check();

            let result_str     =  if let Some(x) = result {i.name.clone()} else {String::new()};
            let address_string =  if let Some(x) = result {x.to_string()} else {".".to_string()};

            output_result = format!("{} {:>12}", output_result, result_str);
            output_detail = format!("{} {}", output_detail, address_string);
        }

        let output  = format!("{} {}            Hosts: {}", dt.format("%Y-%m-%d__%H:%M:%S"), output_result, output_detail);

        

        println!("{}", output);

        if let Some(out) = &mut out {
            out.write(format!("{}\n",output).as_bytes());
        }

        let delay = u64::from_str_radix(matches.get_one::<String>("delay").unwrap_or(&"10".to_string()), 10).unwrap();
        
        thread::sleep(Duration::from_secs(delay));
    }
}
