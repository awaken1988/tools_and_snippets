use std::{net::{IpAddr, Ipv4Addr, Ipv6Addr}, time::Duration, iter};
use dns_lookup::{lookup_host, lookup_addr};
use ping::ping;

struct CheckerState<'a>
{
    idx:  usize,
    addresses: &'a Vec<IpAddr>,

}

impl<'a> CheckerState<'a> {
    fn new(addresses: &'a Vec<IpAddr>) -> CheckerState {
        return CheckerState {
            idx: addresses.len(),
            addresses: addresses,
        };
    }

    fn next_id(mut self) -> usize {
        self.idx+=1;
        if self.idx >= self.addresses.len() {
            self.idx=0;
        }
            
        return self.idx
    } 

    fn check(self)-> bool {
        for i_round in 0..self.addresses.len() {
            let ip_addr = self.addresses[i_round];

            let ping_result = ping(
                ip_addr,
                Some(Duration::from_secs(3)),
                Option::None,
                Option::None,
                Option::None,
                Option::None);

            if let Ok(()) = ping_result {
                return true;
            }
        }

        return false;
    }
}

fn main() {

    let hostnames = vec![
        "www.google.de".to_string(),
        "8.8.8.8".to_string(),
        "8.8.4.4".to_string(),
    ];

    let addresses = hostnames.iter()
        .map(|x|{ lookup_host(x) })
        .map_while(|x| if let Ok(y) = x {Some(y)} else { Option::None }  )
        .flatten()
        .collect::<Vec<IpAddr>>();


    let mut checker = CheckerState::new(&addresses);



    // for i in addresses {
    //        match i {
    //         IpAddr::V4(x) => println!("v4 {}", i),
    //         IpAddr::V6(x) => println!("v6 {}", i),
    //        }
    // }


    // let x = ping(
    //     IpAddr::from([127,0,0,1,]),
    //     Some(Duration::from_secs(3)),
    //     Option::None,
    //     Option::None,
    //     Option::None,
    //     Option::None).unwrap();
    // let result=lookup_host("8.8.8.8").unwrap();
    // for i in result.iter() {
    //     match i {
    //         IpAddr::V4(x) => println!("v4 {}", i),
    //         IpAddr::V6(x) => println!("v6 {}", i),
    //     }
    // }


    println!("Hello, world!");
}
