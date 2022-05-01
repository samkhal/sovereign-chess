import React from "react"
import ChessgroundSovereign from './ChessgroundSovereign';
import { useState, useEffect } from 'react';
import createModule from "./engine.mjs"

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


const initialFen = 'aqabvrvnbrbnbbbqbkbbbnbrynyrsbsq/aranvpvpbpbpbpbpbpbpbpbpypypsnsr/nbnp12opob/nqnp12opoq/crcp12rprr/cncp12rprn/gbgp12pppb/gqgp12pppq/yqyp12vpvq/ybyp12vpvb/onop12npnn/orop12npnr/rqrp12cpcq/rbrp12cpcb/srsnppppwpwpwpwpwpwpwpwpgpgpanar/sqsbprpnwrwnwbwqwkwbwnwrgngrabaq w';
function App() {
  const [getLegalMoves, setGetLegalMoves] = useState(null);
  const [fen, setFen] = useState(initialFen);
  const [turnPlayer, setTurnPlayer] = useState("white");

  useEffect(() => {
    createModule().then((Module) => {
      setGetLegalMoves(() => Module.cwrap('get_legal_moves', 'string', ['string']));
    });
  }, []);

  function handleMove(_orig, _dest, _metadata, fen) {

    let newTurnPlayer = turnPlayer === 'white' ? 'black' : 'white';
    setTurnPlayer(newTurnPlayer);

    let new_fen = fen;
    new_fen += " ";
    new_fen += newTurnPlayer === 'white' ? 'w' : 'b';
    setFen(new_fen);
    setTurnPlayer(newTurnPlayer);
  }

  if (!getLegalMoves) {
    return "Loading..."; // TODO(samkhal)
  }

  return (
    <div className="App">
      <ChessgroundSovereign
        config={{
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
            free: false,
            dests: movelistToDests(getLegalMoves(fen)),
            events: { after: handleMove }
          },
          highlight: {
            lastMove: false,
          }
        }}
      />
    </div>
  );
}

export default App;
