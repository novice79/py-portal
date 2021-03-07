
import moment from "moment";
import _ from 'lodash'
import util from "./util";

window.sss = {}
// this is target socks5 provider peerid
window.socks_pid = '';
// this is target home peerid
window.target_home_pid = '';
class Busi {
    constructor() {
        _.bindAll(this, ['init', 'reg_evt']);
        this.need_check_friends = new Date();
    }
    init() {
        this.reg_evt();
        this.go_pub();      
    }
    go_pub() {
        try {
            const addrs = util.ss_addrs();
            // console.log(`go_pub()------------${JSON.stringify(addrs)}`)
            addrs.forEach(s => {
                if (s.enabled) {
                    // sss[s.addr] = new WSS(s.addr)
                }
            })
        } catch (err) {
            console.log(`go_pub() exception`)
        }
    }
    reg_evt() {
    
        vm.$on("start_socks_server_succeed", data => {
            window.remote_proxy_port = parseInt(data.port)+100;
            console.log(`window.remote_proxy_port=${window.remote_proxy_port}`);
        });
        // {"6dca4b03f180588b468036b1eff907a4":11,"cmd":"noty_proxy_info"}
        vm.$on("add_ss", data => {
            const addr = data.addr;
            if (!sss.hasOwnProperty(addr)) {
                sss[addr] = new WSS(addr)
                vm.$emit('ss_changed', {cmd: 'add_ss', addr});
            }
        });

        vm.$on("del_ss", data => {
            const addr = data.addr;
            if (sss.hasOwnProperty(addr)) {
                sss[addr].destroy();
                delete sss[addr];
                vm.$emit('ss_changed', {cmd: 'del_ss', addr});
            }
        });

      
    }
}


export default new Busi;