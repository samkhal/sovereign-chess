import React from "react"
import { useState, useEffect, useRef } from 'react';
import createModule from "./engine.mjs"
import PromotionDialog from './PromotionDialog';
import { Chessground as NativeChessground } from 'chessground-sovereign'
import { Button, TextField, Switch, FormControlLabel, FormGroup } from '@mui/material';
import { ThemeProvider, createTheme } from '@mui/material/styles';

import "./assets/theme.css"
import "./assets/examples.css"
import "./assets/chessground.css"

const darkTheme = createTheme({
  palette: {
    mode: 'dark',
  },
});

function movelistToDests(movelist) {
  var dests = new Map();
  for (const move of movelist.split(' ')) {
    // temporary hacky, TODO(samkhal) add proper protocol
    const from = move.slice(0, 2);
    const to = move.slice(2, 4);
    if (dests.has(from)) {
      dests.get(from).push(to);
    } else {
      dests.set(from, [to]);
    }
  }
  return dests;
}
export const key2pos = (k) => [k.charCodeAt(0) - 97, k.charCodeAt(1) < 58 ? k.charCodeAt(1) - 49 : k.charCodeAt(1) - 65 + 9];
const turnPlayer = (fen) => fen.split(" ")[1] === 'w' ? 'white' : 'black';
const pieceNames = new Map([
  ['pawn', 'p'],
  ['bishop', 'b'],
  ['knight', 'n'],
  ['rook', 'r'],
  ['queen', 'q'],
  ['king', 'k'],
]);

const initialFen = 'aqabvrvnbrbnbbbqbkbbbnbrynyrsbsq/aranvpvpbpbpbpbpbpbpbpbpypypsnsr/nbnp12opob/nqnp12opoq/crcp12rprr/cncp12rprn/gbgp12pppb/gqgp12pppq/yqyp12vpvq/ybyp12vpvb/onop12npnn/orop12npnr/rqrp12cpcq/rbrp12cpcb/srsnppppwpwpwpwpwpwpwpwpgpgpanar/sqsbprpnwrwnwbwqwkwbwnwrgngrabaq w';

function App() {
  const cg = useRef(null);

  const [engine, setEngine] = useState(null);
  const [fen, setFen] = useState(initialFen);

  // Promotion-related state
  const [promotionDialogColor, setPromotionDialogColor] = useState(undefined);
  const [pendingMove, setPendingMove] = useState();

  // Interface settings
  const [allowIllegalMoves, setAllowIllegalMoves] = useState(false);
  const [respondToMoves, setRespondToMoves] = useState(false);
  const respondToMovesRef = useRef();
  respondToMovesRef.current = respondToMoves;
  const [fullAutoplay, setFullAutoplay] = useState(false);
  const fullAutoplayRef = useRef();
  fullAutoplayRef.current = fullAutoplay;

  const [computerMoveDelay, setComputerMoveDelay] = useState(500);
  const computerMoveDelayRef = useRef();
  computerMoveDelayRef.current = computerMoveDelay;


  useEffect(() => {
    createModule().then((Module) => {
      setEngine({
        getLegalMoves: Module.cwrap('get_legal_moves', 'string', ['string']),
        makeMove: Module.cwrap('make_move', 'string', ['string', 'string']),
        selectMove: Module.cwrap('select_move', 'string', ['string'])
      });
    });
  }, []);

  function handleMove(orig, dest, promotionRole) {

    const destPos = key2pos(dest);
    const piece = cg.current.state.pieces.get(dest);
    // is this a promotion?
    if (piece.role === 'pawn' &&
      destPos[0] > 5 && destPos[0] < 10 &&
      destPos[1] > 5 && destPos[1] < 10) {
      setPendingMove({
        from: orig,
        to: dest,
        color: piece.color
      });
      setPromotionDialogColor(piece.color);
    }
    else {
      // Not a promotion. Update the fen.

      setFen((oldFen) => {
        let move = orig + dest;
        if (promotionRole !== undefined)
          move += pieceNames.get(promotionRole);


        const newFen = engine.makeMove(oldFen, move);
        if (respondToMovesRef.current)
          setTimeout(() => autoplayMove(newFen), computerMoveDelayRef.current);

        return newFen;
      });

    }
  }

  function autoplayMove(fen, forceFullAutoplay) {
    const selectedMove = engine.selectMove(fen);
    const newFen = engine.makeMove(fen, selectedMove);
    console.log("Computer chose move:", selectedMove);
    setFen(newFen);

    if (fullAutoplayRef.current || forceFullAutoplay) {
      console.log("Scheduling autoplay");
      setTimeout(() => autoplayMove(newFen), computerMoveDelayRef.current);
    }
  }

  if (!engine) {
    return "Loading..."; // TODO(samkhal)
  }

  const config = {
    fen: fen,
    turnPlayer: turnPlayer(fen),
    premovable: {
      enabled: false
    },
    drawable: {
      enabled: false
    },
    movable: {
      color: 'both',
      free: allowIllegalMoves,
      dests: movelistToDests(engine.getLegalMoves(fen)),
      events: { after: (from, to) => handleMove(from, to) }
    },
    highlight: {
      lastMove: false,
    }
  };

  if (cg.current) {
    cg.current.set(config);
  }

  function setCgElement(el) {
    if (cg.current) {
      return;
    }
    if (el)
      cg.current = NativeChessground(el, config);
  }

  function handlePromotionSelection(selectedRole) {
    cg.current.setPieces(new Map([
      [pendingMove.from, undefined],
      [pendingMove.to, { role: selectedRole, color: pendingMove.color, promoted: true }]
    ]));
    setPromotionDialogColor(undefined);
    handleMove(pendingMove.from, pendingMove.to, selectedRole);
  }


  return (
    <ThemeProvider theme={darkTheme}>
      <div className="App">
        <div ref={el => setCgElement(el)} />
        <PromotionDialog color={promotionDialogColor} onClick={handlePromotionSelection} />


        <FormGroup>
          <Button
            variant="text"
            onClick={() => setFen(initialFen)}
          >New game</Button>
          <FormControlLabel control={<Switch
            checked={respondToMoves}
            onChange={(e) => setRespondToMoves(e.target.checked)}
          />} label="Computer responds to moves" />
          {<FormControlLabel control={<Switch
            checked={fullAutoplay}
            onChange={(e) => {
              setFullAutoplay(e.target.checked);
              if (e.target.checked)
                autoplayMove(fen, true);
            }}
          />} label="Computer plays both sides" />
          }
          <TextField
            label="Computer move delay"
            variant="standard"
            value={computerMoveDelay}
            onChange={(e) => setComputerMoveDelay(Number(e.target.value))} />
          <FormControlLabel control={<Switch
            checked={allowIllegalMoves}
            onChange={(e) => setAllowIllegalMoves(e.target.checked)}
          />} label="Allow illegal moves" />
          <TextField
            label="FEN"
            variant="standard"
            value={fen}
            onChange={(e) => setFen(e.target.value)} />
        </FormGroup>

      </div>
    </ThemeProvider>
  );
}

export default App;
