<template>
  <div class="store">
    <div class="cur">
      <div>{{cur_dir}}</div>
      <button class="green return" @click="back" :disabled="cur_dir == ''">
        <i class="material-icons">keyboard_backspace</i>
      </button>
      <button class="yellow new-folder" @click="create_folder">
        <i class="material-icons">create_new_folder</i>
      </button>
    </div>
    <div class="main">
      <keep-alive>
        <router-view/>
      </keep-alive>
    </div>
    
    <!-- {{ $t('message') }} -->
    <div class="fs-menu" @click.stop="hide_menu"> 
      <i class="handle small material-icons">menu</i>
      <div class="fs-dropdown">
        <div class="all disabled" @click="switch_page($t('all'), 'all')"><i class="material-icons">store</i>{{$t('all')}}</div>
        <div class="image" @click="switch_page($t('image'), 'image')"><i class="material-icons">image</i>{{$t('image')}}</div>
        <div class="audio" @click="switch_page($t('audio'), 'audio')"><i class="material-icons">audiotrack</i>{{$t('audio')}}</div>
        <div class="video" @click="switch_page($t('video'), 'video')"><i class="material-icons">ondemand_video</i>{{$t('video')}}</div>
        
      </div>
      <div class="sort-dropdown">
        <div @click="set_sort_criteria(1)">
          {{$t('sort-by-time')}}
          <i v-if="sort_type==1 && order_type=='asc'" class="material-icons">arrow_upward</i>
          <i v-else-if="sort_type==1 && order_type=='desc'" class="material-icons">arrow_downward</i>
          <i v-else class="material-icons">swap_vert</i>
        </div>
        <div @click="set_sort_criteria(2)">
          {{$t('sort-by-name')}}
          <i v-if="sort_type==2 && order_type=='asc'" class="material-icons">arrow_upward</i>
          <i v-else-if="sort_type==2 && order_type=='desc'" class="material-icons">arrow_downward</i>
          <i v-else class="material-icons">swap_vert</i>
        </div>
        <div @click="set_sort_criteria(3)">
          {{$t('sort-by-type')}}
          <i v-if="sort_type==3 && order_type=='asc'" class="material-icons">arrow_upward</i>
          <i v-else-if="sort_type==2 && order_type=='desc'" class="material-icons">arrow_downward</i>
          <i v-else class="material-icons">swap_vert</i>
        </div>
        <div class="multi-sel" @click="switch_page($t('multi-sel'), 'multi-sel')">
          {{$t('multi-sel')}}
          <i class="material-icons">select_all</i>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapState, mapGetters, mapMutations, mapActions } from 'vuex'
import util from "@/common/util";


export default {
  name: "store",
  created: function() {

  },
  destroyed() {

  },
  watch: {
    '$route' (to, from) {
      // console.log(`from=${JSON.stringify(from.name)}`)
      // console.log(`to=${JSON.stringify(to.name)}`)
      $(`.fs-menu > div > div`).removeClass('disabled')
      $(`.fs-dropdown > .${to.name}`).addClass('disabled')
    }
  },
  mounted() {
    this.draggie = new Draggabilly('.fs-menu', {
      containment: '.content', 
      handle: '.handle'
    });
    this.draggie.on( 'dragStart', ()=>{
      // $(".fs-menu").removeClass("is-open");
    });
    this.draggie.on( 'staticClick', ()=>{
      $(".fs-menu").toggleClass("is-open");
      // console.log('staticClick');
    });
    const ui_set = db.ui.findOne({})
    this.set_sort_type(ui_set.sort_type)
    this.set_order_type(ui_set.sort_asc)
  },
  data() {
    return {
      placeholder: '',
    };
  },
  computed: {
    ...mapState([
      'sort_type',
      'order_type',
      'files'
    ]),
    ...mapGetters({
      cur_dir: 'path'
    }),

  },
  methods: {
    ...mapMutations([
      'set_sort_type',
      'set_order_type'
    ]),
    ...mapActions([
      'create_dir',
      'back' 
    ]),
    switch_page(sub, name){
      util.switch_page(sub, name)
    },
    set_sort_criteria(st) {
      if(this.sort_type == st){
        this.set_order_type( !this.order_type )
      } else {
        this.set_sort_type(st)
      }
      db.ui.findAndUpdate({}, s => {
        s.sort_type = this.sort_type;
        s.sort_asc = this.order_type;
      });

    },
    hide_menu(){
      // $(".fs-menu").removeClass("is-open");
    },

    create_folder() {
      let name = prompt(this.$t('new-folder-name'), this.$t('new-folder'));
      if(!name) return; //this is cancelled?
      name = name.replace(/[\n\r]/gm, "");
      if(!name) return util.show_alert_top_tm(`${this.$t('new-folder-name')}${this.$t('can-not-be-empty')}`)
      let i = _.findIndex( this.files, f=> f.type == 'dir' && f.name == name );
      if(i >= 0) return util.show_alert_top_tm(this.$t('already-exist'))
      if (name) {
        this.create_dir(name)
      }
    },
    
  }
};
</script>
<style scoped >
.store{
  overflow: hidden;
  position: relative;
}
.handle{
  padding: 0.35em;
}
.handle.material-icons{
  vertical-align: sub; 
  pointer-events: initial;
}
.fs-menu{
  position: absolute;
  /* must use left/top, or it will stretch */
  left: 55%;
  top: 40%;
  border-radius: 2em;
  background-color: rgb(230, 227, 227);
}
.fs-dropdown{
  background-color: lightgray;
  display: none;
  white-space: nowrap;
  position: absolute;
  right: 50%;
  top: 80%;
  margin: 0;
}
.sort-dropdown{
  /* font-size: 1.06em; */
  background-color: lightgray;
  display: none;
  white-space: nowrap;
  position: absolute;
  top: 80%;
  margin: 0;
  left: 50%;
}
.fs-dropdown > div + div, .sort-dropdown > div + div{
  border-top: 1px outset;
}
.fs-dropdown > div, .sort-dropdown > div {
  /* width: 100%; */
  text-align: center;
  padding: 0.8em 1em;
  font-weight: normal;
}
.fs-menu.is-open > div {
  display: block;
}
.cur > div:first-child {
  /* border: 1px solid red; */
  flex: 1;
}
.cur {
  /* position: fixed;
  width: 100%;
  top: 2rem; */
  align-items: center;
  display: flex;
  border: 1px solid;
  text-align: left;
  background-color: rgb(130, 233, 247);
  z-index: 7;
}
.main {
  height: calc(100vh - 6.5rem);
  overflow-y: auto;
}

.return,
.new-folder {
  border: none;
  outline: none;
  background-color: transparent;
  font-weight: 900;
  /* border-radius: 1em; */
  padding: 0.7em;
  margin-left: auto;
}
button:disabled {
  /* background-color: #ccc; */
  color: grey;
}
div.disabled
{
  pointer-events: none;
  /* for "disabled" effect */
  opacity: 0.5;
  background: #CCC;
}

</style>
