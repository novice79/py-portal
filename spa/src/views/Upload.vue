<template>
  <div class="upload">
    <div class="progressbar" v-for="(v, k) in uploading">
      <div v-bind:style="{width: v.progress}"></div>
      <div class="cap">{{`${progress_cap(v)}(${v.progress})`}}</div>
    </div>
    <input type="file" multiple @change="processFile($event)">
    <div v-if="up_count==0">
      <button class="upload_btn" @click.prevent="open_file()">{{$t('upload-file')}}</button>
      <div class="info-panel">
        上传的文件保存在程序的mystore目录下。
      </div>
    </div>
    
  </div>
</template>

<script>

import _ from 'lodash'
import util from "@/common/util";
export default {
  name: "Upload",
  props: {
    msg: String
  },
  created: async function() {},
  destroyed() {},
  mounted() {

  },
  data() {
    return {
      uploading: {},
      up_count: 0
    };
  },
  computed: {
    address() {
      return ``;
    }
  },
  methods: {
    progress_cap(f){
      return `${_.truncate(f.name, {'length': 7})}${util.formatFileSize(f.size)}`
    },
    open_file() {
      $('input[type="file"]').click();
    },
    upload_file(file) {
      let loaded = 0;
      let step = 1048576; //1024*1024; size of one chunk
      let total = file.size; // total size of file
      let start = 0; // starting position
      let reader = new FileReader();
      let blob = file.slice(start, step); //a single chunk in starting of step size
      reader.readAsBinaryString(blob); // reading that chunk. when it read it, onload will be invoked

      reader.onload = e => {
        let flag = 0;
        if (loaded > 0 && loaded + step <= total) flag = 1;
        else if (loaded + step > total) flag = 2;
        let buff = reader.result;
        buff = Buffer.concat([
          Buffer.from(file.name),
          Buffer.from([0], "binary"),
          Buffer.from(buff, "binary"),
          Buffer.from([flag], "binary")
        ]);
        $.ajax({
          type: "POST",
          url: "/upload",
          // timeout: 3000,
          dataType: "text",
          contentType: "application/octet-stream",
          data: buff,
          processData: false
        })
          .done(r => {
            loaded += step; //increasing loaded which is being used as start position for next chunk
            if (loaded <= total) {
              // if file is not completely uploaded
              blob = file.slice(loaded, loaded + step); // getting next chunk
              reader.readAsBinaryString(blob); //reading it through file reader which will call onload again. So it will happen recursively until file is completely uploaded.
            } else {
              // if file is uploaded completely
              loaded = total; // just changed loaded which could be used to show status.
              if (--this.up_count == 0) {
                this.uploading = {};
              }
            }
            if (this.up_count > 0)
              this.uploading[file.name].progress = `${parseFloat(
                (loaded / total) * 100
              ).toFixed(2)}%`;
          })
          .fail(err => {
            console.log("failed: ", err);
          });
      };
    },
    processFile(event) {
      if (event.target.files.length == 0) return;
      this.up_count = event.target.files.length;
      const ups = {};
      _.each(event.target.files, f => {
        ups[f.name] = { progress: "0%", size: f.size, name: f.name };
        this.upload_file(f);
      });
      this.uploading = ups;
      $('input[type="file"]').val("");
    }
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
input[type="file"] {
  display: none;
}
.progressbar {
  position: relative;
  margin: 0.3em 0.5em;
  background-color: black;
  border-radius: 1em; /* (height of inner div) / 2 + padding */
  padding: 0.2em;
  overflow: hidden;
}
.progressbar > .cap {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translateX(-50%) translateY(-50%);
  background-color: initial;
  width: 100%;
  color: green;
  font-weight: bold;
}
.progressbar + .progressbar {
  margin-top: 0.7em;
}
.progressbar > div {
  text-align: center;
  background-color: orange;
  height: 1.5em;
  line-height: 1.5em;
  border-radius: 0.7em;
}
.upload_btn {
  border-radius: 0.7em;
  display: block;
  background-color: rgb(166, 218, 218);
  color: rgb(71, 70, 70);
  font-weight: 900;
  font-size: 1.7rem;
  width: 90%;
  margin: 0.9em auto;
  text-align: center;
}
.info-panel{
  border: 2px inset darkgray;
}
</style>
