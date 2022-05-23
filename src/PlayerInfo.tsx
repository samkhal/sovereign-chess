// Displays info about a player's position
import { Box, Typography } from '@mui/material';
import { Color } from 'chessground-sovereign/types';
import React from 'react';
import PieceImage from './PieceImage';

interface Props {
  playerName: string,
  ownedColor: Color,
  controlledColors: Array<Color>,
  className?: string
}

export default function PlayerInfo(props: Props) {

  return (
    <Box className={props.className + " playerinfo"}>
      <Typography>{props.playerName + ":"}</Typography>
      <PieceImage
        role="king"
        color={props.ownedColor}
        titleText={`${props.playerName} owns the ${props.ownedColor} pieces.`} />
      {props.controlledColors.map((c) =>
        <PieceImage
          role="pawn"
          color={c}
          key={c}
          titleText={`${props.playerName} controls the ${c} pieces.`} />
      )}
    </Box>
  );
}