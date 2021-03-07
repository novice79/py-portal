<template>
  <div class="sss">
    <div v-for="(v, k) in sss" class="server">
      <div class="bubble">{{v.total}}</div>
      <div class="addr">{{k}}</div>
    </div>

  </div>
</template>

<script>

import util from "@/common/util";
export default {
  name: "Sss",
  props: {
    msg: String
  },
  created: async function() {
    this.$root.$on("ss_changed", this.refresh);
    this.$root.$on("online_count", this.refresh);
  },
  destroyed() {
    this.$root.$off("ss_changed", this.refresh);
    this.$root.$off("online_count", this.refresh);
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

    refresh(){
      this.sss = sss;
    },

    remove(s) {

    },
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>

.server{
  background-color: rgb(192, 206, 218);
  border: 1px lightcyan inset;
  width: 100%;
  display: flex;
  margin: 0.7em 0.5em 0.5em 0;
  align-items: center;
}
.addr{
  margin-left: 2em; 
}
.wc {
  margin-left: auto;
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

.sss {
  display: flex;
  /* justify-content: space-between; */
  align-items: center;
  flex-flow: column;
  /* overflow: hidden; */
  font-weight: 700;
}



</style>
