use std::net::{SocketAddr, UdpSocket};
use std::time::{SystemTime, UNIX_EPOCH};
use std::panic;
use serde::{Serialize, Deserialize};

// If a connection does not respond in 33msec (~double the sending frequency) it is removed from the connections table
const CONN_TIMEOUT: u128 = 33;

enum ServerAction {
    Get,
    Push,
}

#[derive(Serialize, Deserialize, Debug)]
struct Coord {
    x: f32,
    y: f32,
}

#[derive(Serialize, Deserialize, Debug)]
struct Snake {
    id: u32,
    coords: Vec<Coord>, 
}

#[derive(Serialize, Deserialize, Debug)]
struct GameState {
    apple_location: Coord,
    players: Vec<Snake>,
}

struct Conn {
    conn_id: u32,
    snake_id: u32,
    addr: SocketAddr,
    ttl: u128,
    creation_time: u128,
}

struct Connections {
    conns: Vec<Conn>,
}
impl Connections {
    fn contains(&self, src: &SocketAddr) -> bool {
        if self.conns.len() == 0 {
            return false;
        }

        for conn in &self.conns {
            if conn.addr == *src {
                return true;
            }
        }
        return true;
    } 

    fn push(&mut self, conn: Conn) {
        self.conns.push(conn);
    }

    fn connect(&mut self, addr: &SocketAddr) {
        if !self.contains(addr) {
            // println!("Adding {} to connections table.", addr);
            let conn = Conn {
                conn_id: 10,
                snake_id: 11,
                addr: *addr,
                ttl: CONN_TIMEOUT,
                creation_time: get_now(),
            };
            self.push(conn);
        }
    }

    fn disconnect(&mut self, addr: SocketAddr) {
        let result = self.conns.iter().position(|c| c.addr == addr);
        match result {
            None        => return,
            Some(index) => {
                self.conns.remove(index);
            }
        }
    }

    fn broadcast(&self, socket: &UdpSocket, state: &GameState) {
        let json_data = serde_json::to_string(&state).unwrap();
        // println!("{json_data}");
        
        // println!();
        for conn in self.conns.iter() {
            // println!("Broadcasting to {}", conn.addr);
            let _ = socket.send_to(json_data.as_bytes(), conn.addr); 
        }
    }

    fn update_ttls(&mut self) {
        let mut addrs_to_remove: Vec<SocketAddr> = vec![];
        for conn in self.conns.iter_mut() {
            let new_ttl = panic::catch_unwind(|| {
                CONN_TIMEOUT - (get_now() - conn.creation_time)
            });
            
            match new_ttl {
                Ok(ttl) => conn.ttl = ttl,
                Err(_)  => addrs_to_remove.push(conn.addr),
            }
        }

        for addr in addrs_to_remove.iter() {
            self.disconnect(*addr);
        }
    }

    fn update_state(
        &self, 
        snake_id: &u32,
        state: &mut GameState, 
        new_coords: Vec<Coord>
    ) {
        let mut index: usize = 0;
        for (i, snake) in state.players.iter().enumerate() {
            if snake.id == *snake_id {
                index = i;
            }
        }
        state.players[index].coords = new_coords;     
    }
}

fn get_now() -> u128 {
    let now = SystemTime::now();
    let milis = now
        .duration_since(UNIX_EPOCH)
        .expect("Time went backwards")
        .as_millis();
    milis
}

fn handle_packet(
    addr: SocketAddr, 
    packet: String, 
    connections: &mut Connections,
    state: &mut GameState,
) -> String {
    let mut action = ServerAction::Get;
    let mut data: Option<Snake> = None;
    let lines: Vec<&str> = packet.lines().collect();
    for (i, val) in lines.iter().enumerate() {
        if i == 0 {
            match *val {
                "GET" => action = ServerAction::Get,
                "PUSH" => action = ServerAction::Push,
                _    => return String::from("401"), 
            }
        }
        else if i == 1 {
            // println!("Authentication step.");
        }
        else {
            match action {
                ServerAction::Get => continue,
                ServerAction::Push => data = serde_json::from_slice(val.as_bytes()).unwrap(),
            }
        }
    }
    
    if !connections.contains(&addr) {
        // authenticate client
        connections.connect(&addr);
    }

    match action {
        ServerAction::Get  => println!("Get action received"),
        ServerAction::Push => {
            let snake_data = data.unwrap();
            connections.update_state(&snake_data.id, state, snake_data.coords)
        },
    }
    return String::from("200");
}

// ----------------- Test Function -----------------------
fn create_dummy_state() -> GameState {
    let apple_loc = Coord {
        x: 0.0,
        y: 0.0,
    };

    let mut players: Vec<Snake> = vec![];
    for i in 0..5 {
        let snake = Snake{
            id: i,
            coords: vec![Coord{ x: 1.0, y: 2.0 }],
        };
        players.push(snake);
    }

    GameState { 
        apple_location: apple_loc,
        players: players,
    }
} // --------------- end test function --------------------


fn main() {
    let socket = UdpSocket::bind("127.0.0.1:8000").unwrap();
    let mut connections = Connections {
        conns: Vec::new(),
    };
    let mut state = create_dummy_state();

    let mut buf = [0; 1024];
    let mut src: SocketAddr;
    let mut n: usize; 
    loop {
        // Receive packet
        (n, src) = socket.recv_from(&mut buf).expect("Error reading data.");
        // println!("Received packet:\n{:#?}", String::from_utf8(buf[..n].to_vec()).unwrap());
        
        // handle packet
        let resp_str = handle_packet(
            src, 
            String::from_utf8(buf[0..n].to_vec()).unwrap(), // TODO: need to handle errors here.
            &mut connections,
            &mut state,
        );
        let response = resp_str.as_bytes();
        let _ = socket.send_to(response, src);

        // broadcast state if changed
        connections.broadcast(&socket, &state);

        // update connection ttls
        connections.update_ttls(); 
    }
}

