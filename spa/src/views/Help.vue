<template>
  <div class="help">
    <!-- 只支持安卓5.1以上版本 -->
    <h2>当前版本:20210304050502</h2>
    <h2>程序功能</h2>
    <ol>
      <li><p>可在局域网内（通过浏览器）上传大文件到手机，可同时上传多个文件, 后上传的同名文件会把之前的覆盖掉</p></li>
      <li><p>上传的zip/rar/7z格式的压缩包可在手机上解压</p></li>
      <li><p>同一局域网内所有浏览器客户端，可同时上传、下载、在线播放音乐、视频</p></li>
      <li><p>支持音乐的单曲循环、顺序播放、目录循环等播放方式</p></li>
      <li><p>支持在手机上视频转码（转为h264+aac+vtt以支持在浏览器中播放）、提取视频声轨为mp3文件、提取字幕文件，或srt字幕转vtt</p></li>
      <li><p>可上传自己的个人nodejs网站到手机运行，支持nodejs直接存取sqlite数据库（内附api说明和操作步骤）</p></li>
      <li><p>带bt下载客户端(通过浏览器访问http://手机ip:9091 , 默认登录用户名/密码=py/bt --可修改)</p></li>
      <li><p>带dlna服务(显示名称为:res-dlna),可在局域网内任何dlna客户端访问,包括智能电视, 手机/PC vlc播放器</p></li>
      <li><p>带发贴功能,各浏览器客户端可创建/浏览 markdown格式文章</p></li>
      <li><p>附带扫码功能</p></li>
    </ol>
    <h2>常见问答</h2>
    <ol>
      <li>
        <h3>支持的安卓版本</h3>
        <p>
        安卓5.1以上的都支持。
        </p>
      </li>
      <li>
        <h3>上传的文件保存到哪里去了？</h3>
        <p>
        所有上传的文件都保存在安卓sd卡的mystore目录。应用不会触及其它目录和文件。
        </p>
      </li>
      <li>
        <h3>上传文件的大小和数量有没限制？</h3>
        <p>
        没限制，只要手机存储容量够大，别写爆了就行。上传速度与 sd卡读写速度，和内网网速有关。
        </p>
      </li>
      <li>
        <h3>其它客户端能删除我的文件吗？</h3>
        <p>
        文件的删除、移动、重命名等操作只在手机服务端开放，客户端只能浏览、下载、播放（用浏览器）音频/视频，和上传文件。
        </p>
      </li>
      <li>
        <h3>文件的上传、下载会消耗我的流量吗？</h3>
        <p>
        文件的上传、下载都是用内网流量（因为必须要连WiFi才能用），不会用到公网流量。与其他玩家交互会用到外网流量。实际上不用插sim卡，只连wifi就可使用。
        </p>
      </li>
      <li>
        <h3>选择图片、音频、视频分类时会显示所有子目录下该类型的文件吗？</h3>
        <p>
        不会显示子目录中的内容，只显示【当前目录】中的文件。
        【当前目录】显示在“文件管理”上面的标题栏，点<i class="yellow material-icons">folder</i>进入子目录，点<i class="green material-icons">keyboard_backspace</i>返回上级目录。
        点右上角的<i class="yellow-new material-icons">create_new_folder</i>在当前目录中新建文件夹。<br>
        <span>提示：</span>如果选择文件分类后，只会显示”当前目录“下该类型的文件，不会显示子文件夹。如果要进入其它文件夹，请先选择”全部“，再点击进入子文件夹。
        </p>
      </li>
      <li>
        <h3>可以改服务的监听端口吗？</h3>
        <p>
        可以，点“服务地址”页面中的端口号，一个手指的青色标记那里，在弹出的对话框中改端口号即可。<br>
        有些绑定的端口不可修改，比如“个人网站”的端口就是在“服务地址”端口上+1，“远程代理”端口在“本地代理”端口上+100，
        还有远程玩家主页端口是在本地“服务地址”端口上+100, 这几个都是随着它绑定端口的修改而修改。
        </p>
      </li>
      <li>
        <h3>怎样使用自定义的pac？</h3>
        <p>
        编写任意内容的pac文件，比如叫:my.pac，上传至mystore目录，然后通过http://你的安卓ip:端口/store/my.pac访问即可。
        </p>
      </li>

    </ol>
  </div>
</template>
<script>
export default {
  name: "Help",
  created: function() {
    
  },
  destroyed() {

  },
  mounted() {

  },
  beforeRouteEnter(to, from, next) {
    if(navigator.language !== 'zh-CN') {
      next('/help-en');
    } else {
      next()
    }
  },
  data() {
    return {
    };
  },
  computed: {

  },
  methods: {
    test(){

    }
  }
};
</script>
<style scoped>
.faq{
  margin-top: 1em;
}

p, h3{
  text-align: left;
  /* margin: 1.0em 0.1em; */
}
span {
  color: red;
}
.to-apply{
  margin: 1.9em 1em;
  font-size: 1.7rem;
}
u{
  margin-left: 1em;
}

</style>