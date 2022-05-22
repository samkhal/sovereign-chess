import { Color, Role } from "chessground-sovereign/types";
import React from "react";

interface Props {
  color: Color,
  role: Role,
  onClick?: () => void
};

export default function PieceImage(props: Props) {
  const pieceName = props.color + " " + props.role;
  const PieceTag = "piece" as keyof JSX.IntrinsicElements;

  return (
    <PieceTag className={pieceName + " promotion"}>
      <svg onClick={() => { if (props.onClick) props.onClick(); }} viewBox="0 0 50 50" xmlns="http://www.w3.org/2000/svg">
        <use className={pieceName} href={'#' + props.role + '-svg'} />
      </svg>
    </PieceTag>
  );
}