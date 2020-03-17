const loadPost = async () => {
  const response = await fetch('content/post/test.md');
  const text = await response.text();

  const post = document.getElementById('post');
  const lines = text.split(/\n|\r\n/);

  var block = undefined;

  const endBlock = () => {
    if (block) {
      post.append(block);
      block = undefined;
    }
  }

  for (const line of lines) {
    if (line.length === 0) {
      endBlock();
    }
    else if (line.startsWith('# ')) {
      endBlock();
      block = document.createElement('H1');
      block.append(line.substring(2));
    }
    else if (line.startsWith('## ')) {
      endBlock();
      block = document.createElement('H2');
      block.append(line.substring(3));
    }
    else if (line.startsWith('### ')) {
      endBlock();
      block = document.createElement('H3');
      block.append(line.substring(4));
    }
    else if (line.startsWith('- ')) {
      if (!block) block = document.createElement('UL');
      if (block.tagName !== 'UL') endBlock();
      const li = document.createElement('LI');
      li.append(line.substring(2));
      block.append(li);
    }
    else if (line.startsWith('> ')) {
      if (!block) block = document.createElement('blockquote');
      if (block.tagName !== 'BLOCKQUOTE') endBlock();
      block.append(line.substring(2) + ' ');
    }
    else if (line.startsWith('    ')) {
      
    }
    else {
      if (!block) block = document.createElement('P');
      if (block.tagName !== 'P') endBlock();
      block.append(line + ' ');
    }
  }

  endBlock();
}

loadPost();