import React from "react"
import { useState, useEffect, useRef } from 'react';
// @ts-ignore
import createModule from "./engine.mjs"
import PromotionDialog from './PromotionDialog';
import PlayerInfo from './PlayerInfo';
import { Chessground as NativeChessground } from 'chessground-sovereign'
import { Button, TextField, Switch, FormControlLabel, FormGroup } from '@mui/material';
import { ThemeProvider, createTheme } from '@mui/material/styles';

import "./assets/theme.css"
import "./assets/app.css"
import "./assets/chessground.css"

import { FEN, Key, Role, Color, Side, Pos } from 'chessground-sovereign/types';

type Move = string; // TODO

interface PendingMove {
  from: Key,
  to: Key,
  color: Color
};

interface Engine {
  getLegalMoves(fen: FEN): Move[];
  makeMove(fen: FEN, move: Move): FEN;
  selectMove(fen: FEN): Move | undefined;
  getOwnedColor(fen: FEN, activePlayer: boolean): Color;
  getControlledColors(fen: FEN, activePlayer: boolean): Color[];
}

const colorCharToName = new Map<string, Color>(
  Object.values(Color).map((value: Color) => [value.charAt(0), value])
);

function wrapModule(Module: EngineModule) {
  const getLegalMoves = Module.cwrap('get_legal_moves', 'string', ['string']);
  const makeMove = Module.cwrap('make_move', 'string', ['string', 'string']);
  const selectMove = Module.cwrap('select_move', 'string', ['string']);
  const getOwnedColor = Module.cwrap('get_owned_color', 'string', ['string', 'boolean']);
  const getControlledColors = Module.cwrap('get_controlled_colors', 'string', ['string', 'boolean']);
  return {
    getLegalMoves: (fen: FEN) => getLegalMoves(fen).split(' '),
    makeMove: makeMove,
    selectMove: (fen: FEN) => { const move = selectMove(fen); return move ? move : undefined; },
    getOwnedColor: (fen: FEN, activePlayer: boolean) => colorCharToName.get(getOwnedColor(fen, activePlayer))!,
    getControlledColors: (fen: FEN, activePlayer: boolean) => {
      const colorStr = getControlledColors(fen, activePlayer)
      if (!colorStr)
        return [];
      return colorStr.split(' ').map((colorChar) => colorCharToName.get(colorChar)!)
    }
  };
}

interface EngineModule extends EmscriptenModule {
  cwrap: typeof cwrap;
}

const darkTheme = createTheme({
  palette: {
    mode: 'dark',
  },
});

function movelistToDests(movelist: Move[]) {
  var dests = new Map();
  for (const move of movelist) {
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
export const key2pos = (k: Key): Pos => [k.charCodeAt(0) - 97, k.charCodeAt(1) < 58 ? k.charCodeAt(1) - 49 : k.charCodeAt(1) - 65 + 9];
const turnPlayer = (fen: FEN): Side => fen.split(" ")[1] === 'w' ? Side.White : Side.Black;
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
  const cg = useRef<ReturnType<typeof NativeChessground> | null>(null);

  const [engine, setEngine] = useState<Engine | null>(null);
  const [fen, setFen] = useState(initialFen);

  // Promotion-related state
  const [promotionDialogColor, setPromotionDialogColor] = useState<Color | undefined>(undefined);
  const [pendingMove, setPendingMove] = useState<PendingMove>();

  // Interface settings
  const [allowIllegalMoves, setAllowIllegalMoves] = useState(false);
  const [respondToMoves, setRespondToMoves] = useState(false);
  const respondToMovesRef = useRef(respondToMoves);
  respondToMovesRef.current = respondToMoves;
  const [fullAutoplay, setFullAutoplay] = useState(false);
  const fullAutoplayRef = useRef(fullAutoplay);
  fullAutoplayRef.current = fullAutoplay;

  const [computerMoveDelay, setComputerMoveDelay] = useState(500);
  const computerMoveDelayRef = useRef(computerMoveDelay);
  computerMoveDelayRef.current = computerMoveDelay;

  useEffect(() => {
    createModule().then((Module: EngineModule) => {
      setEngine(wrapModule(Module));
    });
  }, []);

  function handleMove(orig: Key, dest: Key, promotionRole: Role | undefined = undefined) {

    const destPos = key2pos(dest);
    const piece = cg.current!.state.pieces.get(dest)!;
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


        const newFen = engine!.makeMove(oldFen, move);
        if (respondToMovesRef.current)
          setTimeout(() => autoplayMove(newFen), computerMoveDelayRef.current);

        return newFen;
      });

    }
  }

  function autoplayMove(fen: FEN, forceFullAutoplay: boolean = false) {
    const selectedMove = engine!.selectMove(fen);
    if (selectedMove === undefined) {
      console.log("No legal moves");
      return;
    }
    const newFen = engine!.makeMove(fen, selectedMove);
    console.log("Computer chose move:", selectedMove);
    setFen(newFen);

    if (fullAutoplayRef.current || forceFullAutoplay) {
      console.log("Scheduling autoplay");
      setTimeout(() => autoplayMove(newFen), computerMoveDelayRef.current);
    }
  }

  if (!engine) {
    return <div>"Loading..."</div>; // TODO(samkhal)
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
      color: 'both' as const,
      free: allowIllegalMoves,
      dests: movelistToDests(engine.getLegalMoves(fen)),
      events: { after: (from: Key, to: Key) => handleMove(from, to) }
    },
    highlight: {
      lastMove: false,
    }
  };

  if (cg.current) {
    cg.current.set(config);
  }

  function setCgElement(el: HTMLElement | null) {
    if (cg.current) {
      return;
    }
    if (el)
      cg.current = NativeChessground(el, config);
  }

  function handlePromotionSelection(selectedRole: Role) {
    if (pendingMove) {
      cg.current!.setPieces(new Map([
        [pendingMove.from, undefined],
        [pendingMove.to, { role: selectedRole, color: pendingMove.color, promoted: true }]
      ]));
      setPromotionDialogColor(undefined);
      handleMove(pendingMove.from, pendingMove.to, selectedRole);
    }
  }


  // TODO fix this
  const selfIsActivePlayer = turnPlayer(fen) === Side.White;

  return (
    <ThemeProvider theme={darkTheme}>
      <div className="App">
        <PlayerInfo
          className="playerinfo-enemy"
          playerName="Enemy"
          ownedColor={engine.getOwnedColor(fen, !selfIsActivePlayer)}
          controlledColors={engine.getControlledColors(fen, !selfIsActivePlayer)} />
        <div className="blue merida board-outer">
          <div ref={el => setCgElement(el)} />
        </div>
        <PlayerInfo
          className="playerinfo-self"
          playerName="Player"
          ownedColor={engine.getOwnedColor(fen, selfIsActivePlayer)}
          controlledColors={engine.getControlledColors(fen, selfIsActivePlayer)} />

        <PromotionDialog color={promotionDialogColor} onClick={handlePromotionSelection} />


        <FormGroup className="settings">
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
