const solution = [
    [2, 3, 6, 5, 9, 7, 1, 4, 8],
    [9, 4, 1, 6, 3, 8, 2, 5, 7],
    [5, 8, 7, 4, 1, 2, 9, 6, 3],
    [3, 1, 9, 8, 7, 5, 6, 2, 4],
    [4, 6, 8, 3, 2, 9, 5, 7, 1],
    [7, 5, 2, 1, 6, 4, 8, 3, 9],
    [1, 2, 5, 9, 4, 3, 7, 8, 6],
    [8, 9, 3, 7, 5, 6, 4, 1, 2],
    [6, 7, 4, 2, 8, 1, 3, 9, 5],
  ];

let num_cheats = 0;

function checkPuzzle() {
  const table = document.querySelector("table");
  let solved = true;
  for (let i = 0; i < table.rows.length; i++) {
    for (let j = 0; j < table.rows[0].cells.length; j++) {
      let cell = table.rows[i].cells[j];
      if (cell.firstChild.value !== solution[i][j].toString() &&
          cell.innerText !== solution[i][j].toString()) {
        solved = false;
      }
    }
  }
  let output = document.querySelector("#puzzle-output");
  if (solved) {
    output.innerHTML = "<p style='color:green'>Yay good job!"
    document.querySelector("#success").hidden = false;
  } else {
    output.innerHTML = "<p style='color:red'>Sorry!  Try again."
  }
}

function solvePuzzle() {
  let table = document.querySelector("table");
  for (let i = 0; i < table.rows.length; i++) {
    for (let j = 0; j < table.rows[0].cells.length; j++) {
      let input = table.rows[i].cells[j].querySelector("input");
      if (input) {
        input.value = solution[i][j].toString();
      }
    }
  }
}

function solveOneCell() {
  const table = document.querySelector("table");
  let unsolved = [];
  for (let i = 0; i < table.rows.length; i++) {
    for (let j = 0; j < table.rows[0].cells.length; j++) {
      const input = table.rows[i].cells[j].querySelector("input");
      if (input && input.value !== solution[i][j].toString()) {
        unsolved.push([i, j]);
      }
    }
  }
  if (unsolved.length > 0) {
    const num = Math.floor(Math.random() * unsolved.length);
    const i = unsolved[num][0];
    const j = unsolved[num][1];
    const input = table.rows[i].cells[j].querySelector("input");
    input.value = solution[i][j].toString();
  }
}

function cheat() {
  const max_cheats = 5;
  const cheat_strings = [
    "<p>I knew you'd click this. Eve going for the forbidden fruit.",
    "<p>Don't be naughty!",
    "<p>Did you cheat your way up Langley?",
    "<p>What would your parents say?",
    "<p>Probably like, 'suck it Dems - show em Lauren'.",
    "<p>You are persistent, gotta give you that.",
    "<p>Some people just want to watch the world burn.",
    "<p>You know, you'll get arthritis clicking this much.",
    ];
  let output = document.querySelector("#puzzle-output");
  if (num_cheats < cheat_strings.length) {
    output.innerHTML = cheat_strings[num_cheats];
  } else if (num_cheats == cheat_strings.length) {
    output.innerHTML = "Ok fine. But only cause you're cute.";
    solveOneCell();
  } else if (num_cheats < cheat_strings.length + max_cheats) {
    output.innerHTML = "I won't let you do this forever.";
    solveOneCell();
  } else {
    output.innerHTML = "That's it. I'm putting the hammer down.";
  }
  num_cheats++;
}