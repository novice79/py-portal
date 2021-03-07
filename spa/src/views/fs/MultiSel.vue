<template>
  <div class="multi-sel">
    <div class="op-btn">
      <button @click="back">{{$t('back')}}</button>
      <button @click="del_file" :disabled="!is_selected">{{$t('delete')}}</button>
      <button @click="move_to" :disabled="!is_selected">{{$t('move')}}</button>
    </div>
    <div class="op-btn-dummy"></div>
    <!-- {{ $t('message') }} -->
    <div class="fi" v-for="f in files">
      <input type="checkbox" v-model="f.sel">
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
import util from "@/common/util";

export default {
  name: "multi-sel",
  created: function() {
    this.$root.$on("update_file_list", this.update_file_list);
  },
  destroyed() {
    this.$root.$off("update_file_list", this.update_file_list);
  },
  mounted() {
    this.files = g.files;
  },
  data() {
    return {
      files: []
    };
  },
  computed: {
    selected_file_names() {
      let sels = _.filter(this.files, f=>f.sel)
      sels = _.map(sels, 'name');
      return sels.join(" + ");
    },
    is_selected() {
      const sels = _.filter(this.files, f=>f.sel)
      return sels.length > 0;
    }
  },
  methods: {
    back(){
      this.$root.$emit('back', '');
    },
    open_file_or_folder(f) {
      if (f.type == 'dir') {
        this.$root.$emit("enter_dir", f.name);
      } 
    },
    update_file_list(files) {
      this.files = files;
    },
    
    move_to() {
      // alert(i18n.t("hello"));
      this.$root.$emit('move_to', '');
    },
    
    del_file() {
      const r = confirm(`${this.$t('confirm-del')}${this.selected_file_names}`)
      if(r){
        for(let f of this.files){
          if(f.sel){
            const cmd = {
              cmd: "del_file",
              path: f.path
            };
            ws.send(JSON.stringify(cmd));
          }
        }
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
