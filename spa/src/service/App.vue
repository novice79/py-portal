<template>
  <div id="app">
    <h1>Service</h1>
  </div>
</template>

<script>

export default {
  name: 'Service',
  created: function() {
    this.test()
  },
  beforeDestroy() {},
  destroyed() {

  },
  mounted() {},
  methods: {
    async test() {
      const self = this;
      setTimeout(()=>{
        console.log("from webview service ..................");
        self.test()
      }, 2000)
    },
    toggle_menu() {
 
    },


    
  }
}
</script>

<style scoped>

</style>
