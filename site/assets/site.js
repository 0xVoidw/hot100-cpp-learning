/* gen_site.js */
(function(){
  function ls(){try{return JSON.parse(localStorage.getItem('hot100_done')||'[]')}catch(e){return[]}}
  function set(a){localStorage.setItem('hot100_done',JSON.stringify(a))}
  function theme(){return localStorage.getItem('hot100_theme')||'light'}
  function applyTheme(){document.documentElement.setAttribute('data-theme',theme())}
  applyTheme();
  document.addEventListener('DOMContentLoaded',function(){
    var t=document.getElementById('themeBtn');
    if(t)t.addEventListener('click',function(){localStorage.setItem('hot100_theme',theme()==='dark'?'light':'dark');applyTheme();t.textContent=theme()==='dark'?'☀':'🌙';});
    // 进度
    var done=ls();
    function update(){var all=document.querySelectorAll('[data-num]');var d=0;all.forEach(function(e){if(done.indexOf(e.getAttribute('data-num'))>=0)d++});var pb=document.getElementById('progressbar');if(pb){var pct=all.length?Math.round(d/all.length*100):0;pb.style.width=pct+'%';var l=document.getElementById('progresslabel');if(l)l.textContent=pct+'% ('+d+'/'+all.length+')';}}
    document.querySelectorAll('[data-check]').forEach(function(cb){cb.addEventListener('change',function(){var n=cb.getAttribute('data-check');done=ls();if(cb.checked){if(done.indexOf(n)<0)done.push(n)}else{done=done.filter(function(x){return x!==n})}set(done);update();})});
    // 搜索/筛选
    var sq=document.getElementById('search');
    var ds=document.querySelectorAll('.chip[data-diff]');
    var cs=document.querySelectorAll('.chip[data-cat]');
    var diff='',cat='',txt='';
    function match(){document.querySelectorAll('.card').forEach(function(c){
      var okDiff=!diff||c.getAttribute('data-diff')===diff;
      var okCat=!cat||c.getAttribute('data-cat')===cat;
      var hay=(c.getAttribute('data-s')||'').toLowerCase();
      var okTxt=!txt||hay.indexOf(txt)>=0;
      c.style.display=(okDiff&&okCat&&okTxt)?'':'none';
    });}
    if(sq)sq.addEventListener('input',function(){txt=sq.value.trim().toLowerCase();match()});
    ds.forEach(function(b){b.addEventListener('click',function(){diff=diff===b.getAttribute('data-diff')?'':b.getAttribute('data-diff');ds.forEach(function(x){x.classList.remove('on')});if(diff)b.classList.add('on');match()})});
    cs.forEach(function(b){b.addEventListener('click',function(){cat=cat===b.getAttribute('data-cat')?'':b.getAttribute('data-cat');cs.forEach(function(x){x.classList.remove('on')});if(cat)b.classList.add('on');match()})});
    update();
  });
})();
