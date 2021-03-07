<template>
  <div class="outer">
    <div v-for="s in sss" class="server">
      <div class="bubble">{{s.count}}</div>
      <input v-model="s.addr" readonly="true" />
      <div @click="enable_svr_or_not(s)">
        <i v-if="s.enabled" class="small material-icons">check_box</i>
        <i v-else class="small material-icons">check_box_outline_blank</i>
      </div>
      <div>&nbsp;&nbsp;&nbsp;</div>
      <div @click="remove(s)"><i class="small material-icons">delete</i></div>
    </div>
    <button @click="add_server"><i class="material-icons">add</i>&nbsp;{{$t('add')}}</button>
    <!-- <h3>{{$t('how-to-setup-ss')}}&#x2753;</h3> -->
    <p>
      <!-- {{$t('how-to-setup-ss-content')}} -->
      如果没配外网服务器地址，会自动插入测试服地址，您可禁用或改为自己的地址。<br>
      也可设置多台服务器，同时与各台服务器上的玩家进行交互。<br>
      自行搭建服务器请参考： <a href="https://github.com/novice79/ss.git" target="_blank" class="green">这里</a>
    </p>
  </div>
</template>

<script>

import util from "@/common/util";
export default {
  name: "Outer",
  props: {
    msg: String
  },
  created: async function() {
    this.$root.$on("online_count", this.refresh_count);
  },
  destroyed() {
    this.$root.$off("online_count", this.refresh_count);
  },
  mounted() {
    this.refresh();
  },
  data() {
    return {
      sss: [],
    };
  },
  computed: {
    address() {
      
    }
  },
  methods: {
    refresh_count(data){
      const the_ss = this.sss.find(s=>s.addr === data.addr);
      if(the_ss) the_ss.count = data.total;
      
    },
    add_server(){
      let addr = prompt(this.$t('svr-addr'), '');
      if(!addr) return;
      // check addr format
      if( !util.check_addr(addr) ) return util.show_alert_top_tm( this.$t('invalid-format') )
      const the_ss = this.sss.find(s=>s.addr === addr);
      if( the_ss ) return util.show_alert_top_tm( this.$t('already-exist') )
      db.ss.insert({ addr, enabled: true });
      this.refresh();
      this.$root.$emit("add_ss", {addr});
    },
    refresh(){
      this.sss = util.ss_addrs().map(s=>{
        if(sss[s.addr] && sss[s.addr].total){
          s.count = sss[s.addr].total;
        } else {
          s.count = 0;
        }       
        return s;
      })
    },
    enable_svr_or_not(s) {
      let title;
      if(s.enabled){
        title = this.$t('confirm-disable')
      } else {
        title = this.$t('confirm-enable')
      }
      const r = confirm( `[${s.addr}]？`);
      if(r){
        s.enabled = !s.enabled;
        db.ss.update(s);
        if(s.enabled){
          this.$root.$emit("add_ss", {addr:s.addr});
        }else{
          this.$root.$emit("del_ss", {addr:s.addr});
          s.count = 0;
        }        
      }
    },
    remove(s) {
      const r = confirm(`${this.$t('confirm-del')}[${s.addr}]？`)
      if(r){
        db.ss.remove(s);
        this.refresh();
        this.$root.$emit("del_ss", {addr:s.addr});
      }
    },
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>

.server{
  display: flex;
  margin: 0.7em 0.5em 0.5em 0;
  align-items: center;
}
input {
  /* flex: 1; */
  /* this necessary */
  min-width: 5em;
  width: 100%;
  flex-grow: 1;
  font-size: 1.3rem;
}
p {
  margin: 0.7em 1.7em;
  text-align: left;
}
.bubble{
  background: #666;
  border-radius: 50%;
  color: #fff;
  display: inline-block;
  font-weight: bold;
  line-height: 2.7em;
  text-align: center;
  min-width: 2.7em; 
}

.outer {
  display: flex;
  /* justify-content: space-between; */
  align-items: center;
  flex-flow: column;
  /* overflow: hidden; */
  font-weight: 700;
}
button {
  width: 80%;
  border-radius: 1em;
  font-size: 1.2em;
  /* padding: 0 1em; */
  margin: 1.0em 0.5em;
}


</style>
