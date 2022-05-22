// Displays info about a player's position
import Box from '@mui/material/Box';
import { Color } from 'chessground-sovereign/types';
import React from 'react';
import PieceImage from './PieceImage';

interface Props {
  ownedColor: Color,
  controlledColors: Array<Color>,
  className?: string
}

export default function PlayerInfo(props: Props) {

  return (
    <Box className={props.className + " playerinfo"}>
      <PieceImage role="king" color={props.ownedColor} />
      {props.controlledColors.map((c) =>
        <PieceImage role="pawn" color={c} key={c} />)}
    </Box>
  );
}