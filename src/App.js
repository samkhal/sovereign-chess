import React from "react"
import { useState, useEffect, useRef } from 'react';
import createModule from "./engine.mjs"
import PromotionDialog from './PromotionDialog';
import { Chessground as NativeChessground } from 'chessground-sovereign'
import { TextField, Switch, FormControlLabel, FormGroup } from '@mui/material';

import "./assets/theme.css"
import "./assets/examples.css"
import "./assets/chessground.css"

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

const initialFen = 'aqabvrvnbrbnbbbqbkbbbnbrynyrsbsq/aranvpvpbpbpbpbpbpbpbpbpypypsnsr/nbnp12opob/nqnp12opoq/crcp12rprr/cncp12rprn/gbgp12pppb/gqgp12pppq/yqyp12vpvq/ybyp12vpvb/onop12npnn/orop12npnr/rqrp12cpcq/rbrp12cpcb/srsnppppwpwpwpwpwpwpwpwpgpgpanar/sqsbprpnwrwnwbwqwkwbwnwrgngrabaq w';
function App() {
  const cg = useRef(null);

  const [getLegalMoves, setGetLegalMoves] = useState(null);
  const [fen, setFen] = useState(initialFen);
  const [turnPlayer, setTurnPlayer] = useState("white");

  // Promotion-related state
  const [promotionDialogColor, setPromotionDialogColor] = useState(undefined);
  const [pendingMove, setPendingMove] = useState();

  // Interface settings
  const [allowIllegalMoves, setAllowIllegalMoves] = useState(false);

  useEffect(() => {
    createModule().then((Module) => {
      setGetLegalMoves(() => Module.cwrap('get_legal_moves', 'string', ['string']));
    });
  }, []);

  function handleMove(orig, dest) {

    // is this a promotion?
    const destPos = key2pos(dest);
    const piece = cg.current.state.pieces.get(dest);
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

      let newTurnPlayer = turnPlayer === 'white' ? 'black' : 'white';
      setTurnPlayer(newTurnPlayer);

      let new_fen = cg.current.getFen();
      new_fen += " ";
      new_fen += newTurnPlayer === 'white' ? 'w' : 'b';
      setFen(new_fen);
      setTurnPlayer(newTurnPlayer);
    }
  }

  if (!getLegalMoves) {
    return "Loading..."; // TODO(samkhal)
  }

  const config = {
    fen: fen,
    turnPlayer: turnPlayer,
    premovable: {
      enabled: false
    },
    drawable: {
      enabled: false
    },
    movable: {
      color: 'both',
      free: allowIllegalMoves,
      dests: movelistToDests(getLegalMoves(fen)),
      events: { after: handleMove }
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
    handleMove(pendingMove.from, pendingMove.to);
  }


  return (
    <div className="App">
      <div ref={el => setCgElement(el)} />
      <PromotionDialog color={promotionDialogColor} onClick={handlePromotionSelection} />


      <FormGroup>
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
  );
}

export default App;
