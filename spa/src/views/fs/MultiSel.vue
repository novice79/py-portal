<template>
  <div class="multi-sel">
    <div class="op-btn">
      <button @click="restore_before_move">{{$t('back')}}</button>
      <button @click="del_file" :disabled="!is_selected">{{$t('delete')}}</button>
      <button @click="move_to" :disabled="!is_selected">{{$t('move')}}</button>
    </div>
    <div class="op-btn-dummy"></div>
    <!-- {{ $t('message') }} -->
    <div class="fi" :key="f.path" v-for="f in files">
      <input type="checkbox" v-model="selected[f.name]">
      <div class="file-desc">
        <div class="fn">{{f.name}}</div>
        <div class="file-time">
          <div>{{f.time}}</div>
          <div class="file-size" v-if="f.type != 'dir'">{{formatFileSize(f.size)}}</div>
          <!-- <div>{{f.path}}</div> -->
        </div>        
      </div>
      <div v-if="f.type=='dir'" @click="open_file_or_folder(f)">
        <i class="small material-icons">folder</i>
      </div>
      <div v-else-if="f.type.includes('image/')">
        <i class="small material-icons">image</i>
      </div>
      <div v-else-if="f.type.includes('audio/')">
        <i class="small material-icons">audiotrack</i>
      </div>
      <div v-else-if="f.type.includes('video/')">
        <i class="small material-icons">ondemand_video</i>
      </div>
      <div v-else>
        <i class="small material-icons">insert_drive_file</i>
      </div>
    </div>
    <div class="pad-bottom"></div>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapGetters, mapActions } from 'vuex'
import util from "@/common/util";

export default {
  name: "multi-sel",
  created: function() {

  },
  destroyed() {

  },
  mounted() {

  },
  data() {
    return {
      selected: []
    };
  },
  watch: {
    'files' (new_value, old_value) {
      this.selected = {}
    }
  },
  computed: {
    ...mapGetters({
      files: 'all',
    }),
    selected_file_names() {
      let sels = _.pickBy(this.selected)
      sels = _.keys(sels);
      return sels.join(" + ");
    },
    is_selected() {
      return this.sel_files().length > 0;
    }
  },
  methods: {
    ...mapActions([
      'create_dir',
      'enter',
      'back',
      'delete',
      'move_to',
      'confirm_move',
      'restore_before_move'
    ]),
    sel_files(){
      return _.keys( _.pickBy(this.selected) )
    },
    open_file_or_folder(f) {
      if (f.type == 'dir') {
        this.enter(f.name);
      } 
    },

    move_to() {
      // alert(i18n.t("hello"));
      this.move_to( this.sel_files() );
    },
    
    del_file() {
      const r = confirm(`${this.$t('confirm-del')}${this.selected_file_names}`)
      if(r){
        this.delete( this.sel_files() );
      }
    },
    file_url(file_path) {
      return util.path2url(file_path);
    },
    formatFileSize(bytes, decimalPoint) {
      return util.formatFileSize(bytes, decimalPoint)
    }
  }
};
</script>
<style scoped >
.multi-sel{
  min-height: 97%;
}
.file-desc {
  flex: 1;
  margin: 0 0.4em;
}

.op-dropdown {
  background-color: lightgray;
  display: none;
  white-space: nowrap;
  position: absolute;
  right: 2.3em;
  top: 1.2em;
  margin: 0;
  z-index: 79;
}

.file-size{
  margin-left: auto;
}
.file-time {
  display: flex;
  font-size: 0.7rem;
}
.fn{
  word-break: break-all;
}
.fi {
  display: flex;
  align-items: center;
  text-align: left;
  background-color: rgb(177, 250, 250);
  margin: 0.3em 0;
  border: 2px inset black;
}
button:disabled {
  /* background-color: #ccc; */
  color: grey;
}

input[type="checkbox"]{
  min-height: 1.9em;
  min-width: 1.9em;
  margin-right: 1.7em;
}
button{
  font-size: 1.5rem;
  border-radius: 1em;
  padding: 0.3em;
}
.op-btn-dummy{
  min-height: 3.1em;
}
.op-btn{
  position: absolute;
  /* top: -0.4em; */
  width: 100%;

  display: flex;
  justify-content: space-around;
  background-color: rgb(174, 209, 206);
  opacity: 0.9;
}
</style>
