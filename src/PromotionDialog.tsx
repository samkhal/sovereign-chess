import Modal from '@mui/material/Modal';
import Box from '@mui/material/Box';
import PieceImage from './PieceImage';
import { Color, Role } from "chessground-sovereign/types";
import React from 'react';

interface Props {
  color: Color | undefined,
  onClick: (role: Role) => void
};

export default function PromotionDialog(props: Props) {
  const style = {
    position: 'absolute',
    top: '50%',
    left: '50%',
    transform: 'translate(-50%, -50%)',
    width: 400,
    bgcolor: 'background.paper',
    border: '2px solid #000',
    boxShadow: 24,
    p: 4,
  };

  return (
    <Modal open={props.color !== undefined}>
      <Box sx={style} className="promotion-panel">
        <PieceImage role="knight" color={props.color!} onClick={() => props.onClick("knight")} />
        <PieceImage role="bishop" color={props.color!} onClick={() => props.onClick("bishop")} />
        <PieceImage role="rook" color={props.color!} onClick={() => props.onClick("rook")} />
        <PieceImage role="queen" color={props.color!} onClick={() => props.onClick("queen")} />
      </Box>
    </Modal>
  );
}